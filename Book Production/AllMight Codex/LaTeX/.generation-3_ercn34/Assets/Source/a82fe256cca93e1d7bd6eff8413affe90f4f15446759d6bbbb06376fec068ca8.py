#!/usr/bin/env python3
"""Compile exact book excerpts; prove ordinary behavior AND its throwing-move gap."""
import argparse
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile

# Parse the actual Markdown blocks, rather than copying a different ECS model.
sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from note_integrity import code_blocks

ROOT = Path(__file__).resolve().parents[2]
BOOK = ROOT / 'Continuous Notes/Game Engine - From First Frame to Living Worlds.md'

DRIVER = r"""
#include <iostream>
struct ThrowingComponent {
    int value;
    inline static bool fail = false;
    explicit ThrowingComponent(int v) : value(v) {}
    ThrowingComponent(const ThrowingComponent&) = default;
    ThrowingComponent(ThrowingComponent&&) noexcept = default;
    ThrowingComponent& operator=(ThrowingComponent&& other) {
        // Throw before touching the value, so any broken entity metadata is
        // attributable to Remove's ordering, not a partially assigned T.
        if (fail) throw std::runtime_error("deliberate component move failure");
        value = other.value;
        return *this;
    }
};

int main() {
    SparseSet<int> pool;
    const Entity old{0, 1}, second{1, 1}, replacement{0, 2};
    pool.Emplace(old, 7);
    pool.Emplace(second, 9);
    if (pool.Get(old) != 7 || pool.Get(second) != 9) return 1;
    if (!pool.Remove(old)) return 2;
    // Swap-removal must repair the surviving entity's reverse map.
    if (pool.Contains(old) || pool.Get(second) != 9 || pool.Size() != 1) return 3;
    pool.Emplace(replacement, 11);
    if (pool.Contains(old) || pool.Get(replacement) != 11) return 4;

    SparseSet<ThrowingComponent> unsafe;
    unsafe.Emplace(old, 10);
    unsafe.Emplace(second, 20);
    ThrowingComponent::fail = true;
    bool caught = false;
    try { unsafe.Remove(old); }
    catch (const std::runtime_error&) { caught = true; }
    ThrowingComponent::fail = false;
    // This EXPECTED counterexample is evidence of an unsupported T contract.
    // Passing this check does not make the current generic Remove safe.
    if (!caught || unsafe.Entities().size() != 2 ||
        unsafe.Entities()[0] != second || unsafe.Entities()[1] != second) return 5;
    if (unsafe.Contains(old)) return 6;
    std::cout << "PASS: normal swap-remove and generation reuse; "
                 "EXPECTED throwing-move invariant failure reproduced\n";
}
"""


def assemble(book):
    blocks = [b.body for b in code_blocks(book.read_text()) if b.language == 'cpp']
    # Matching exact declarations keeps introductory sketches out of this check.
    entities = [b for b in blocks if 'struct Entity final' in b and '#include <cstdint>' in b]
    pools = [b for b in blocks if 'class SparseSet' in b and
             'std::is_nothrow_move_assignable_v<T>' in b and 'EnsureSparse' in b]
    if len(entities) != 1 or len(pools) != 1:
        raise ValueError('Expected one exact Entity and full SparseSet excerpt')
    # The book fragment omits a direct header; add it explicitly for this TU.
    return '#include <type_traits>\n' + entities[0] + '\n' + pools[0] + '\n' + DRIVER


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('book', nargs='?', type=Path, default=BOOK)
    parser.add_argument('--compiler', default='c++')
    args = parser.parse_args()
    compiler = shutil.which(args.compiler)
    if not compiler:
        parser.error('A C++20 compiler is required')
    with tempfile.TemporaryDirectory(prefix='allmight-ecs-excerpt-') as folder:
        source = Path(folder) / 'ecs.cpp'
        binary = Path(folder) / 'ecs'
        source.write_text(assemble(args.book))
        command = [compiler, '-std=c++20', '-O2', '-DNDEBUG', '-Wall',
                   '-Wextra', '-Wpedantic', '-Werror', str(source), '-o', str(binary)]
        subprocess.run(command, check=True)
        subprocess.run([str(binary)], check=True)


if __name__ == '__main__':
    main()
