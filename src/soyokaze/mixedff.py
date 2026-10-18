from pathlib import Path

def has_mixed_line_endings(file_path, chunk_size=65536):
    line_endings = set()

    try:
        with open(file_path, "rb") as f:
            prev_cr = False

            while True:
                chunk = f.read(chunk_size)
                if not chunk:
                    break

                for b in chunk:

                    if prev_cr:
                        if b == 0x0A:
                            line_endings.add("CRLF")
                            prev_cr = False
                            continue
                        else:
                            line_endings.add("CR")
                            prev_cr = False

                    if b == 0x0D:
                        prev_cr = True

                    elif b == 0x0A:
                        line_endings.add("LF")

                    if len(line_endings) > 1:
                        return True

            if prev_cr:
                line_endings.add("CR")

        return len(line_endings) > 1

    except Exception:
        return False


for path in Path(".").rglob("*"):
    if path.is_file():
        if has_mixed_line_endings(path):
            print(path)
