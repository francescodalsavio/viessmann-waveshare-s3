#!/usr/bin/env python3
"""
VISLA Sniffer Game — interactive Modbus ASCII bus sniffer.

Tells you which action to perform on the original Viessmann master,
captures frames in real-time, decodes them and logs everything for
later analysis.

Usage: python3 tools/sniffer.py [/dev/cu.usbmodemXXX]
"""
import sys
import time
import json
import threading
import queue
import re
from datetime import datetime
from pathlib import Path

try:
    import serial
except ImportError:
    print("ERRORE: pyserial non installato. Esegui: pip3 install pyserial")
    sys.exit(1)

PORT = sys.argv[1] if len(sys.argv) > 1 else "/dev/cu.usbmodem1101"
BAUD = 115200
LOG_FILE = Path(__file__).parent.parent / "sniffer_log.json"

# ANSI colors
C_RESET = "\033[0m"
C_BOLD = "\033[1m"
C_CYAN = "\033[96m"
C_YELLOW = "\033[93m"
C_GREEN = "\033[92m"
C_RED = "\033[91m"
C_GRAY = "\033[90m"
C_MAGENTA = "\033[95m"

# Preset test actions
ACTIONS = [
    ("Accendi il master (ON / esci da standby)", "power_on"),
    ("Spegni il master (OFF / standby)", "power_off"),
    ("Cambia modalità a CALDO", "mode_hot"),
    ("Cambia modalità a FREDDO", "mode_cold"),
    ("Imposta temperatura a 20.0°C", "temp_20"),
    ("Imposta temperatura a 22.0°C", "temp_22"),
    ("Imposta temperatura a 25.0°C", "temp_25"),
    ("Premi temperatura + (una volta)", "temp_up"),
    ("Premi temperatura - (una volta)", "temp_down"),
    ("Cambia velocità ventola: AUTO", "fan_auto"),
    ("Cambia velocità ventola: MIN", "fan_min"),
    ("Cambia velocità ventola: NIGHT", "fan_night"),
    ("Cambia velocità ventola: MAX", "fan_max"),
    ("Azione personalizzata (la descrivi tu)", "custom"),
]

# Frame parser
FRAME_RE = re.compile(r"\[BUS\] :([0-9A-F]+)")
SCREEN_HEAD = "=" * 60


def parse_frame(hex_str):
    """Parse a Modbus ASCII frame body (without : and LRC check)."""
    try:
        data = bytes.fromhex(hex_str)
        if len(data) < 8:
            return None
        addr, func = data[0], data[1]
        reg = (data[2] << 8) | data[3]
        val = (data[4] << 8) | data[5]
        lrc = data[6]
        # Verify LRC
        s = sum(data[:6]) & 0xFF
        lrc_calc = ((-s) & 0xFF)
        ok = lrc == lrc_calc
        return {
            "addr": addr,
            "func": func,
            "reg": reg,
            "val": val,
            "lrc_ok": ok,
            "raw": hex_str,
        }
    except Exception:
        return None


def decode_reg(reg, val):
    """Pretty-print register value with our current understanding."""
    if reg == 101:
        bits = []
        if val & (1 << 14): bits.append("COLD")
        if val & (1 << 13): bits.append("HOT")
        if val & (1 << 7):  bits.append("STANDBY")
        fan = val & 0x03
        fan_names = {0: "fan_off", 1: "fan_min", 2: "fan_auto", 3: "fan_max"}
        bits.append(fan_names.get(fan, f"fan?={fan}"))
        return f"reg101 (config) = 0x{val:04X}  [{', '.join(bits)}]"
    elif reg == 102:
        return f"reg102 (setpoint A) = 0x{val:04X}  ({val/10:.1f}°C)"
    elif reg == 103:
        if val == 0x7FFF:
            return f"reg103 = 0x7FFF  [PLACEHOLDER / no value]"
        return f"reg103 (setpoint B?) = 0x{val:04X}  ({val/10:.1f}°C)"
    return f"reg{reg} = 0x{val:04X}"


class SerialReader(threading.Thread):
    def __init__(self, port, baud):
        super().__init__(daemon=True)
        self.port = port
        self.baud = baud
        self.q = queue.Queue()
        self.stop_flag = threading.Event()
        self.ser = None

    def run(self):
        try:
            self.ser = serial.Serial(self.port, self.baud, timeout=0.2)
        except Exception as e:
            print(f"{C_RED}Errore apertura seriale: {e}{C_RESET}")
            return
        while not self.stop_flag.is_set():
            try:
                line = self.ser.readline()
                if line:
                    self.q.put(line.decode("utf-8", errors="replace").rstrip())
            except Exception:
                pass

    def stop(self):
        self.stop_flag.set()
        if self.ser:
            try:
                self.ser.close()
            except Exception:
                pass


def capture_until_enter(reader, action_name):
    """Drain queue, wait for ENTER, then collect all frames."""
    # Drain any old data
    while not reader.q.empty():
        try: reader.q.get_nowait()
        except queue.Empty: break

    print(f"\n{C_BOLD}{C_YELLOW}>>> AZIONE: {action_name}{C_RESET}")
    print(f"{C_GRAY}    Esegui l'azione sul master, poi premi ENTER quando hai finito.{C_RESET}")
    input(f"{C_CYAN}    [premi ENTER] {C_RESET}")

    # Drain everything captured during the wait
    captured = []
    while not reader.q.empty():
        try:
            line = reader.q.get_nowait()
            captured.append(line)
        except queue.Empty:
            break
    return captured


def extract_frames(lines):
    frames = []
    for ln in lines:
        m = FRAME_RE.search(ln)
        if m:
            f = parse_frame(m.group(1))
            if f:
                frames.append(f)
    return frames


def show_menu():
    print(f"\n{C_BOLD}{C_CYAN}{SCREEN_HEAD}")
    print(" VISLA SNIFFER — scegli azione")
    print(f"{SCREEN_HEAD}{C_RESET}")
    for i, (desc, _) in enumerate(ACTIONS, 1):
        print(f"  {C_BOLD}{i:2d}{C_RESET}) {desc}")
    print(f"  {C_BOLD} 0{C_RESET}) {C_RED}Esci e salva log{C_RESET}")
    return input(f"\n{C_CYAN}> {C_RESET}").strip()


def load_log():
    if LOG_FILE.exists():
        try:
            return json.loads(LOG_FILE.read_text())
        except Exception:
            pass
    return {"sessions": []}


def save_log(log):
    LOG_FILE.write_text(json.dumps(log, indent=2))
    print(f"{C_GREEN}✓ Log salvato in {LOG_FILE}{C_RESET}")


def main():
    print(f"{C_BOLD}{C_MAGENTA}")
    print("╔══════════════════════════════════════════════════════════╗")
    print("║   VISLA SNIFFER GAME — Viessmann Modbus Reverse Engine   ║")
    print("╚══════════════════════════════════════════════════════════╝")
    print(C_RESET)
    print(f"Porta seriale: {C_CYAN}{PORT}{C_RESET}")
    print(f"Log file:      {C_CYAN}{LOG_FILE}{C_RESET}\n")

    reader = SerialReader(PORT, BAUD)
    reader.start()
    time.sleep(1)

    if reader.ser is None:
        print(f"{C_RED}Impossibile aprire la seriale, esco.{C_RESET}")
        return

    print(f"{C_GREEN}✓ Seriale aperta, in ascolto...{C_RESET}")

    log = load_log()
    session = {
        "started": datetime.now().isoformat(),
        "tests": [],
    }

    last_state = {}

    while True:
        choice = show_menu()
        if choice == "0":
            break
        try:
            idx = int(choice) - 1
            if not (0 <= idx < len(ACTIONS)):
                print(f"{C_RED}Scelta non valida{C_RESET}")
                continue
        except ValueError:
            print(f"{C_RED}Inserisci un numero{C_RESET}")
            continue

        desc, key = ACTIONS[idx]
        if key == "custom":
            desc = input(f"{C_CYAN}Descrizione azione: {C_RESET}").strip() or "custom"

        lines = capture_until_enter(reader, desc)
        frames = extract_frames(lines)

        print(f"\n{C_BOLD}--- Risultato ---{C_RESET}")
        print(f"{C_GRAY}{len(lines)} righe seriale, {len(frames)} frame Modbus catturate{C_RESET}\n")

        if not frames:
            print(f"{C_YELLOW}⚠ Nessuna frame catturata. Il master non ha trasmesso?{C_RESET}")
        else:
            for f in frames:
                marker = f"{C_GREEN}✓{C_RESET}" if f["lrc_ok"] else f"{C_RED}✗LRC{C_RESET}"
                decoded = decode_reg(f["reg"], f["val"])
                old_val = last_state.get(f["reg"])
                changed = ""
                if old_val is not None and old_val != f["val"]:
                    changed = f"  {C_YELLOW}(era 0x{old_val:04X}){C_RESET}"
                print(f"  {marker} :{f['raw']}  →  {decoded}{changed}")
                last_state[f["reg"]] = f["val"]

        session["tests"].append({
            "action": desc,
            "key": key,
            "timestamp": datetime.now().isoformat(),
            "frames": frames,
            "raw_lines": lines,
        })

    reader.stop()
    session["ended"] = datetime.now().isoformat()
    log["sessions"].append(session)
    save_log(log)
    print(f"\n{C_BOLD}{C_GREEN}Sessione conclusa. {len(session['tests'])} test registrati.{C_RESET}")


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print(f"\n{C_YELLOW}Interrotto dall'utente{C_RESET}")
