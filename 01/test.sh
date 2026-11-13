#!/usr/bin/env bash

set -u

SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)
PROGRAM="$SCRIPT_DIR/file_copy"
DEMO_DIR="$SCRIPT_DIR/demo"
PASSED=0
FAILED=0

pass() {
    printf 'PASS: %s\n' "$1"
    PASSED=$((PASSED + 1))
}

fail() {
    printf 'FAIL: %s\n' "$1" >&2
    FAILED=$((FAILED + 1))
}

section() {
    printf '\n============================================================\n'
    printf '%s\n' "$1"
    printf '============================================================\n'
}

show_file() {
    local path=$1
    local relative_path=${path#"$DEMO_DIR/"}
    local size

    size=$(stat -c '%s' "$path")
    printf '  %-55s %8s байт\n' "$relative_path" "$size"
}

check_copy() {
    local source=$1
    local copy=$2
    local description=$3

    if [[ ! -f "$copy" ]]; then
        fail "$description: копия не создана"
        return
    fi

    local source_size
    local copy_size

    source_size=$(stat -c '%s' "$source")
    copy_size=$(stat -c '%s' "$copy")

    if cmp -s "$source" "$copy"; then
        pass "$description: исходный=$source_size, копия=$copy_size, содержимое совпадает"
    else
        fail "$description: исходный=$source_size, копия=$copy_size, содержимое различается"
    fi
}

# Каталог содержит только автоматически созданные демонстрационные данные.
rm -rf -- "$DEMO_DIR"
mkdir -p "$DEMO_DIR/unnamed" "$DEMO_DIR/named"

printf 'First line\nSecond line\n' > "$DEMO_DIR/unnamed/text.txt"
: > "$DEMO_DIR/unnamed/empty.txt"
printf '\x00\x01\x02\x7f\x80\xff' > "$DEMO_DIR/unnamed/binary.bin"
dd if=/dev/zero of="$DEMO_DIR/unnamed/large.bin" bs=10000 count=1 status=none
printf 'File with spaces\n' > "$DEMO_DIR/unnamed/file with spaces.txt"

UNNAMED_MISSING='missing.txt'

section 'ТЕСТ 1. Копирование через неименованные каналы'

printf 'Подготовленные исходные файлы:\n'
show_file "$DEMO_DIR/unnamed/text.txt"
show_file "$DEMO_DIR/unnamed/empty.txt"
show_file "$DEMO_DIR/unnamed/binary.bin"
show_file "$DEMO_DIR/unnamed/large.bin"
show_file "$DEMO_DIR/unnamed/file with spaces.txt"
printf '  %-55s %s\n' "unnamed/$UNNAMED_MISSING" 'НЕ СУЩЕСТВУЕТ'

printf '\nКоманда:\n'
printf '  ./file_copy %s text.txt empty.txt binary.bin large.bin "file with spaces.txt"\n' "$UNNAMED_MISSING"
printf '\nВывод программы:\n'

if (cd "$DEMO_DIR/unnamed" && \
    "$PROGRAM" "$UNNAMED_MISSING" text.txt empty.txt binary.bin large.bin "file with spaces.txt"); then
    fail 'ожидался ненулевой код из-за отсутствующего файла'
fi

printf '\nПроверка созданных копий:\n'
check_copy "$DEMO_DIR/unnamed/text.txt" "$DEMO_DIR/unnamed/text.txt.copy" 'text.txt'
check_copy "$DEMO_DIR/unnamed/empty.txt" "$DEMO_DIR/unnamed/empty.txt.copy" 'empty.txt'
check_copy "$DEMO_DIR/unnamed/binary.bin" "$DEMO_DIR/unnamed/binary.bin.copy" 'binary.bin'
check_copy "$DEMO_DIR/unnamed/large.bin" "$DEMO_DIR/unnamed/large.bin.copy" 'large.bin'
check_copy "$DEMO_DIR/unnamed/file with spaces.txt" "$DEMO_DIR/unnamed/file with spaces.txt.copy" \
    'файл с пробелами в имени'

NAMED_TEXT='note.txt'
NAMED_LARGE='data.bin'
NAMED_EMPTY='empty.txt'
NAMED_MISSING='missing.txt'

printf ' /\\_/\\\n( o.o )\n > ^ <\n' > "$DEMO_DIR/named/$NAMED_TEXT"
dd if=/dev/zero of="$DEMO_DIR/named/$NAMED_LARGE" bs=12000 count=1 status=none
: > "$DEMO_DIR/named/$NAMED_EMPTY"

section 'ТЕСТ 2. Копирование через именованные FIFO'

printf 'Подготовленные исходные файлы:\n'
show_file "$DEMO_DIR/named/$NAMED_TEXT"
show_file "$DEMO_DIR/named/$NAMED_LARGE"
show_file "$DEMO_DIR/named/$NAMED_EMPTY"
printf '  %-55s %s\n' "named/$NAMED_MISSING" 'НЕ СУЩЕСТВУЕТ'

printf '\nКоманда:\n'
printf '  ./file_copy -p channel %s %s %s %s\n' \
    "$NAMED_MISSING" "$NAMED_TEXT" "$NAMED_LARGE" "$NAMED_EMPTY"
printf '\nВывод программы:\n'

if (cd "$DEMO_DIR/named" && \
    "$PROGRAM" -p channel "$NAMED_MISSING" "$NAMED_TEXT" "$NAMED_LARGE" "$NAMED_EMPTY"); then
    fail 'ожидался ненулевой код из-за отсутствующего файла'
fi

printf '\nПроверка созданных копий:\n'
check_copy "$DEMO_DIR/named/$NAMED_TEXT" "$DEMO_DIR/named/$NAMED_TEXT.copy" "$NAMED_TEXT"
check_copy "$DEMO_DIR/named/$NAMED_LARGE" "$DEMO_DIR/named/$NAMED_LARGE.copy" "$NAMED_LARGE"
check_copy "$DEMO_DIR/named/$NAMED_EMPTY" "$DEMO_DIR/named/$NAMED_EMPTY.copy" "$NAMED_EMPTY"

section 'ИТОГ'

printf 'Созданные демонстрационные файлы:\n'
find "$DEMO_DIR" -maxdepth 2 -type f -printf '  %P — %s байт\n' | sort

printf '\nПройдено: %d, провалено: %d\n' "$PASSED" "$FAILED"
printf 'Данные для ручной проверки оставлены в: %s\n' "$DEMO_DIR"

if ((FAILED > 0)); then
    exit 1
fi
