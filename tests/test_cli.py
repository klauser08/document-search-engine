"""CLI regression tests. Run from the repository root; no third-party packages required."""

import pathlib
import subprocess
import sys
import tempfile
import unittest

EXECUTABLE = pathlib.Path(sys.argv[1]).resolve()


def run_cli(input_text, cwd=None):
    return subprocess.run(
        [str(EXECUTABLE)], input=input_text, text=True,
        capture_output=True, timeout=3, cwd=cwd,
    )


class CliTests(unittest.TestCase):
    def test_exit_at_each_stage(self):
        for stage in range(4):
            with self.subTest(stage=stage):
                result = run_cli("next\n" * stage + "exit\n")
                self.assertEqual(result.returncode, 0)
                self.assertIn("Goodbye!", result.stdout)
                self.assertEqual(result.stdout.count("Enter query:"), stage + 1)

    def test_eof_at_each_stage(self):
        for stage in range(4):
            with self.subTest(stage=stage):
                result = run_cli("next\n" * stage)
                self.assertEqual(result.returncode, 0)
                self.assertIn("Goodbye!", result.stdout)

    def test_search_and_cache(self):
        result = run_cli("binary search tree\nbinary search tree\nzzzxxyy\nexit\n")
        self.assertEqual(result.returncode, 0)
        self.assertIn("Loaded 20 documents", result.stdout)
        self.assertIn("[CACHE MISS]", result.stdout)
        self.assertIn("[CACHE HIT]", result.stdout)
        self.assertIn("A binary search tree maintains", result.stdout)
        self.assertIn("No relevant document found.", result.stdout)

    def test_normal_search_mode(self):
        result = run_cli("next\nnext\nnext\nbinary search tree\nexit\n")
        self.assertEqual(result.returncode, 0)
        self.assertIn("Demo complete", result.stdout)
        self.assertIn("[CACHE MISS]", result.stdout)

    def test_missing_documents(self):
        with tempfile.TemporaryDirectory() as directory:
            result = run_cli("", cwd=directory)
        self.assertEqual(result.returncode, 1)
        self.assertIn("could not open documents.txt", result.stdout)


if __name__ == "__main__":
    unittest.main(argv=[sys.argv[0]])
