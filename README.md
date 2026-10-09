# cpp-container-runtime

Учебный проект (курсовая работа): собственный контейнерный рантайм на C++ —
упрощённый аналог [runc](https://github.com/opencontainers/runc), легковесного
CLI-инструмента для создания и запуска контейнеров в Linux.

Утилита запускает приложение в изолированном окружении средствами ядра Linux:
отдельная файловая система, сеть и список процессов (namespaces), ограничивает
потребление CPU и памяти (cgroups v2) и выводит метрики работы контейнера для
мониторинга.

> Рабочее название рантайма: **catbox** 🐈‍⬛ (опционально, можно поменять).

## Почему эта тема

- Позволяет изнутри изучить устройство контейнеризации: namespaces, cgroups,
  capabilities, seccomp, pivot_root — то, что Docker прячет «под капотом».
- Сочетает системную разработку на C++ с DevOps-практиками (OCI, мониторинг).
- См. подробный обзор: [docs/01-overview.md](docs/01-overview.md).

## Цели проекта

1. Изучить механизмы изоляции ядра Linux (namespaces, cgroups v2, capabilities, seccomp).
2. Провести сравнительный анализ существующих рантаймов (runc, crun, youki, LXC, gVisor).
3. Спроектировать и реализовать CLI-утилиту на C++:
   - изоляция процесса через namespaces (mnt, pid, uts, ipc, net, user);
   - собственный rootfs через `pivot_root` (минимальный rootfs на busybox);
   - лимиты CPU/памяти через cgroups v2;
   - сбор и вывод метрик контейнера (`cpu.stat`, `memory.current`, `memory.peak`).
4. Протестировать изоляцию и лимиты, сравнить накладные расходы с runc/Docker.

## Структура репозитория

```
cpp-container-runtime/
├── README.md               ← вы здесь
├── docs/
│   ├── 01-overview.md      ← обзор: зачем это нужно, анализ существующих решений
│   └── 02-referat-plan.md  ← план реферата
└── src/                    ← исходный код рантайма (появится позже)
```

## Roadmap

| Веха | Содержание |
|------|-----------|
| v0.1 | Запуск процесса в namespaces (mnt, pid, uts, ipc) + `pivot_root` в rootfs на busybox |
| v0.2 | Лимиты CPU/памяти через cgroups v2 + вывод метрик |
| v0.3 | Сеть (veth + bridge или rootless-вариант), capabilities, seccomp |
| v0.4 | Совместимость с OCI runtime-spec (config.json, жизненный цикл) |
| —    | Реферат и подготовка к защите |

## Требования (для будущей разработки)

- Linux, ядро ≥ 5.x, cgroups v2 (unified hierarchy)
- Компилятор C++20 (GCC/Clang), CMake
- Для тестов rootfs: `busybox`
