# Flyff Resource Framework  
## Phase 1 – Framework Core

---

## Überblick

Das **Flyff Resource Framework** ist ein Framework zur Analyse, Extraktion, Normalisierung und Transformation von Legacy-Daten aus dem Flyff-Client, den Resource-Dateien sowie dem bestehenden Quellcode.

Alle vorhandenen Daten und Strukturen werden ausschließlich als **Input** betrachtet.  
Ziel von Phase 1 ist es, diese Legacy-Strukturen vollständig von ihrer historischen Form zu lösen und in ein **eigenständiges, modernes Datenmodell** zu überführen.

Das Framework dient nicht der Erweiterung bestehender Systeme, sondern dem Aufbau einer **neuen, sauberen Grundlage** für spätere Client-, Server- und Tool-Architekturen.

---

## Ziel von Phase 1

Phase 1 stellt den **Framework Core** bereit.

### Ziele:
- deterministische Transformation von Legacy-Daten
- Normalisierung aller relevanten Informationen
- Aufbau eines eigenständigen internen Datenmodells
- klare Trennung von Domänen, Orchestrierung und Infrastruktur
- reproduzierbare Verarbeitung ohne implizite Abhängigkeiten

### Nicht-Ziele:
- kein Editor
- kein Gameplay
- kein lauffähiger Client oder Server
- keine Live-Systeme
- keine direkte Bearbeitung von Legacy-Dateien

---

## Repository-Struktur (Phase 1)

```
src/
 ├─ app/
 │   └─ main.cpp
 │
 ├─ core/
 │   ├─ asset/
 │   ├─ resource/
 │   ├─ source/
 │   ├─ runtime/
 │   │
 │   ├─ controller/
 │   │   ├─ DataController
 │   │   ├─ PipelineController
 │   │   └─ pipeline/
 │   │       ├─ AssetPipeline
 │   │       ├─ ResourcePipeline
 │   │       ├─ SourcePipeline
 │   │       └─ RuntimePipeline
 │   │
 │   ├─ context/
 │   ├─ log/
 │   ├─ task/
 │   ├─ tests/
 │   └─ ConfigManager
 │
 ├─ data/
 │   ├─ raw/
 │   ├─ asset/
 │   ├─ resource/
 │   ├─ source/
 │   ├─ context/
 │   ├─ module/
 │   ├─ editor/
 │   ├─ global/
 │   ├─ state/
 │   └─ project/
 │
 ├─ plugins/
 │   ├─ api/
 │   └─ host/
 │
 └─ ui/
```

---

## Build

Das Projekt verwendet **CMake** mit Out-of-Source-Builds:

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

---
