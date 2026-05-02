import argparse
import sys
import time
from pathlib import Path

import keyboard
import mido


NOTE_MIN = 36
NOTE_MAX = 96

KEY_MAP = {
    36: "1", 37: "shift+1", 38: "2", 39: "shift+2", 40: "3", 41: "4",
    42: "shift+4", 43: "5", 44: "shift+5", 45: "6", 46: "shift+6", 47: "7",
    48: "8", 49: "shift+8", 50: "9", 51: "shift+9", 52: "0", 53: "q",
    54: "shift+q", 55: "w", 56: "shift+w", 57: "e", 58: "shift+e", 59: "r",
    60: "t", 61: "shift+t", 62: "y", 63: "shift+y", 64: "u", 65: "i",
    66: "shift+i", 67: "o", 68: "shift+o", 69: "p", 70: "shift+p", 71: "a",
    72: "s", 73: "shift+s", 74: "d", 75: "shift+d", 76: "f", 77: "g",
    78: "shift+g", 79: "h", 80: "shift+h", 81: "j", 82: "shift+j", 83: "k",
    84: "l", 85: "shift+l", 86: "z", 87: "shift+z", 88: "x", 89: "c",
    90: "shift+c", 91: "v", 92: "shift+v", 93: "b", 94: "shift+b", 95: "n",
    96: "m",
}


def fit_note_to_piano(note):
    while note > NOTE_MAX:
        note -= 12
    while note < NOTE_MIN:
        note += 12
    return note


def interruptible_sleep(seconds, stop_key):
    deadline = time.monotonic() + max(0.0, seconds)
    while time.monotonic() < deadline:
        if keyboard.is_pressed(stop_key):
            return False
        time.sleep(min(0.01, deadline - time.monotonic()))
    return True


def default_midi_file():
    files = sorted(Path.cwd().glob("*.mid*"))
    return files[0] if files else None


def play_midi(file_path, start_key="f1", stop_key="f10", speed=1.0):
    try:
        midi = mido.MidiFile(file_path)
    except Exception as exc:
        print(f"Could not load MIDI file: {exc}", flush=True)
        return 1

    print(f"Loaded: {file_path}", flush=True)
    print("Focus Roblox and sit at the piano.", flush=True)
    print(f"Press {start_key.upper()} to start. Press {stop_key.upper()} to stop.", flush=True)

    keyboard.wait(start_key)
    print("Playing...", flush=True)

    try:
        for msg in midi:
            if not interruptible_sleep(msg.time / speed, stop_key):
                print("Stopped.", flush=True)
                return 0

            if keyboard.is_pressed(stop_key):
                print("Stopped.", flush=True)
                return 0

            if msg.type == "note_on" and msg.velocity > 0:
                note = fit_note_to_piano(msg.note)
                hotkey = KEY_MAP.get(note)
                if hotkey:
                    keyboard.press_and_release(hotkey)
    except KeyboardInterrupt:
        print("Stopped.", flush=True)
    finally:
        keyboard.release("shift")
        keyboard.release("ctrl")
        keyboard.release("alt")
        keyboard.unhook_all()

    print("Finished.", flush=True)
    return 0


def parse_args():
    fallback = default_midi_file()
    parser = argparse.ArgumentParser(description="Roblox MIDI keyboard backend.")
    parser.add_argument("file", nargs="?", default=fallback, help="Path to a MIDI file.")
    parser.add_argument("--start-key", default="f1", help="Start hotkey.")
    parser.add_argument("--stop-key", default="f10", help="Stop hotkey.")
    parser.add_argument("--speed", type=float, default=1.0, help="Playback speed multiplier.")
    return parser.parse_args()


def main():
    args = parse_args()
    if args.file is None:
        file_path = input("Enter MIDI file path: ").strip().strip('"').strip("'")
    else:
        file_path = str(args.file)

    path = Path(file_path).expanduser()
    if not path.exists():
        print(f"File not found: {path}", flush=True)
        return 1

    if args.speed <= 0:
        print("Speed must be greater than zero.", flush=True)
        return 1

    return play_midi(path, args.start_key.lower(), args.stop_key.lower(), args.speed)


if __name__ == "__main__":
    sys.exit(main())
