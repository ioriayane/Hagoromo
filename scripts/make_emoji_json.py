"""
Convert emoji-test.txt files to JSON format.
https://unicode.org/emoji/charts/emoji-list.html
https://unicode.org/Public/emoji/16.0/emoji-test.txt
"""

import json
import sys


class EmojiTextConverter:
    """
    A class to convert emoji text files to JSON format.
    """

    def __init__(self):
        self._current_group = None
        self._current_subgroup = None

    def parse_comment_line(self, line: str) -> None:
        """
        Parse a comment line and return the description.
        """
        parts = line[1:].strip().split(":")
        if len(parts) != 2:
            return
        if parts[0] == "group":
            print("Group:",  parts[1])
            self._current_group = parts[1].strip()
        elif parts[0] == "subgroup":
            # print("Subgroup:",  parts[1])
            self._current_subgroup = parts[1].strip()

    def parse_emoji_line(self, line: str) -> dict:
        """
        Parse a line containing an emoji and its description.
        """
        state = 0 # 0: code, 1: type, 2: emoji
        parts = line.split(" ")
        codes: list = []
        status: str = ""
        emoji: str = ""
        description: str = ""
        count = 0
        for part in parts:
            count += 1
            part = part.strip()
            if part == ";":
                state = 1
                continue
            elif part == "#":
                state = 2
                continue
            if len(part) == 0:
                continue
            if state == 0:
                codes.append(part)
            elif state == 1:
                status = part
            elif state == 2:
                emoji = part
                break

        description = " ".join(parts[count+1:]).strip()
        is_skin_tone = ("skin tone" in description)

        return {
            "emoji": emoji,
            "status": status,
            "description": description,
            "is_skin_tone": is_skin_tone,
        }

    def parse_emoji_text(self, emoji_text: str) -> list:
        """
        Parse the emoji text and return a dictionary with
        the emoji as the key and its description as the value.
        """
        emoji_array: list = []
        emoji_group_dict: dict = {
            "name": self._current_group,
            "emojis": [],
        }
        lines = emoji_text.splitlines()
        for line in lines:
            line = line.strip()
            if line == "":
                continue

            if line.startswith("#"):
                self.parse_comment_line(line)
                if emoji_group_dict.get("name") != self._current_group:
                    if len(emoji_group_dict.get("emojis")) > 0:
                        emoji_array.append(emoji_group_dict)
                        emoji_group_dict = {
                            "name": self._current_group,
                            "emojis": [],
                        }
                    emoji_group_dict["name"] = self._current_group
            else:
                emoji_data = self.parse_emoji_line(line)
                if len(emoji_data["emoji"]) > 0 \
                        and emoji_data["status"] == "fully-qualified" \
                        and not emoji_data["is_skin_tone"]:
                    emoji_data_dict = {
                        "emoji": emoji_data["emoji"],
                        "description": emoji_data["description"],
                    }
                    emoji_group_dict["emojis"].append(emoji_data_dict)

        return emoji_array

    def load_emoji_text(self, filepath) -> bool:
        """
        Load the emoji text file and return its content as a string.
        """
        with open(filepath, "r", encoding="utf-8") as file:
            return file.read()
        return None


def main(file_path: str):
    """
    Main function to load the emoji text file, parse it, and save it as a JSON file.
    """
    converter = EmojiTextConverter()
    emoji_text = converter.load_emoji_text(file_path)
    if emoji_text is None:
        print("Failed to load emoji-text.txt")
        return

    emoji_array = converter.parse_emoji_text(emoji_text)
    with open("emoji.json", "w", encoding="utf-8") as file:
        json.dump(emoji_array, file, ensure_ascii=False, indent=4)
    print("emoji.json created successfully.")


if __name__ == "__main__":

    arguments = sys.argv[1:]
    if len(arguments) != 1:
        print("Usage: python make_emoji_json.py <path_to_emoji_text_file>")
        sys.exit(1)
    main(arguments[0])
