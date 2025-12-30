# Flyff Resource Framework  
## Phase 1 – Framework Core

---

## Überblick

Das **Flyff Resource Framework** ist ein grundlegendes Framework zur Extraktion, Normalisierung und Modernisierung von Legacy-Datenstrukturen aus dem Flyff-Client, den Resources sowie dem Quellcode.

Dieses Projekt verfolgt **nicht** das Ziel, bestehende Legacy-Systeme zu erweitern oder zu reparieren.  
Stattdessen werden alle vorhandenen Daten ausschließlich als **Input** betrachtet und in ein **eigenständiges, normalisiertes Datenmodell** überführt, das vollständig von historischen Altlasten entkoppelt ist.

Phase 1 konzentriert sich ausschließlich auf den **Framework Core**.

---

## Motivation

Dieses Framework ist in erster Linie ein **persönliches Langzeitprojekt** mit dem Ziel, eine moderne Grundlage für die Entwicklung eines eigenen Spiels zu schaffen.

Die ursprünglichen Flyff-Daten- und Code-Strukturen sind über viele Jahre gewachsen und tragen erhebliche technische Altlasten mit sich.  
Diese Altlasten verhindern eine saubere Erweiterung und Weiterentwicklung.

Das Flyff Resource Framework löst dieses Problem, indem es:

- Legacy-Strukturen vollständig entkoppelt
- Daten deterministisch normalisiert
- ein stabiles Fundament für neue Client- und Server-Architekturen schafft

---

## Umfang von Phase 1 – Framework Core

**Phase 1 beinhaltet:**
- deterministische Extraktions- und Verarbeitungspipelines
- ein eigenständiges, normalisiertes Datenmodell
- klare Verantwortungsbereiche
- eine frameworkorientierte Architektur

**Phase 1 beinhaltet explizit NICHT:**
- einen Editor
- UI-Systeme
- Gameplay- oder Runtime-Systeme
- direkte Bearbeitung von Legacy-Daten

Editor-Funktionalität ist für **Phase 2** vorgesehen.

---

## Repository-Struktur

```
External/
src/
 ├─ app/
 │   └─ main.cpp
 ├─ core/
 │   ├─ pipelines
 │   ├─ controllers
 │   ├─ config
 │   ├─ log
 │   ├─ task
 │   └─ tests
 ├─ asset/
 │   └─ pipeline
 └─ controller/
     ├─ data
     └─ pipeline
```

---

## Externe Abhängigkeiten

Das Verzeichnis `External/` enthält frei verfügbare Drittanbieter-Bibliotheken, die als technische Grundlage dienen:

- STB
- SDL2
- Enlowman
- ImGui

Diese Bibliotheken bilden das Fundament für zukünftige grafische und toolbasierte Erweiterungen.

---

## Kernarchitektur

### Anwendungseinstieg
`src/app/main.cpp`  
Enthält ausschließlich den Einstiegspunkt der Anwendung.  
Es befindet sich keine Framework-Logik in dieser Datei.

---

### Core-Systeme (`src/core`)

Der Core-Bereich enthält alle zentralen Systeme des Frameworks:

- Asset Pipeline
- Resource Pipeline
- Source Pipeline
- Runtime Pipeline
- Konfigurationsverwaltung
- Logging-System
- Task-System (Vorbereitung für Multithreading)

#### Logging
Eigenständiges Logging-System mit:
- Info
- Warning
- Error

#### Task-System
Grundlage für zukünftige Multithreading-Unterstützung.  
Aktuell vorhanden, jedoch noch nicht aktiv genutzt.

---

## Pipeline-Architektur

Das Framework basiert auf **klar getrennten, deterministischen Pipelines**.

### Vorhandene Pipelines
- Resource Pipeline
- Asset Pipeline
- Source Pipeline
- Runtime Pipeline

Jede Pipeline:
- ist in sich abgeschlossen
- kapselt ihre komplette Logik
- erzeugt reproduzierbare Ergebnisse
- gibt keine Logik an den Data Controller zurück

---

### Asset Pipeline (Minimalimplementierung)

Die Asset Pipeline verarbeitet clientseitige Assets wie Texturen, Icons und Sprites.

Verarbeitungsschritte:
1. Indexierung und Kategorisierung
2. technische Klassifizierung
3. Laden der Daten
4. Dekodierung
5. Entschlüsselung (falls notwendig)
6. Parsing (bei binären Formaten)
7. Analyse
8. Normalisierung
9. Konvertierung in Formate, die kompatibel sind mit:
   - Blender
   - Photoshop
   - (optional) Unity

Die Pipeline erzeugt Assets, die mit professionellen Tools weiterverarbeitet werden können.

---

## Controller

### Data Controller
Der Data Controller ist bewusst sehr schlank gehalten.

Aufgaben:
- Orchestrierung der Phasen
- Aufruf der jeweiligen Pipelines

Er enthält **keine**:
- Pipeline-Logik
- Datenverarbeitung
- Endzustände

---

### Pipeline Controller
Jede Pipeline wird durch einen eigenen Pipeline Controller gesteuert.

Grundregel:
> **Jeder Pipeline-Step darf exakt eine Funktion ausführen.**

Dieses Prinzip ermöglicht:
- klare Verantwortlichkeiten
- einfache Wartbarkeit
- zukünftige Multithreading-Fähigkeit

---

## Architekturprinzipien

Das Framework folgt festen, nicht verhandelbaren Regeln:

- eine Verantwortung pro Komponente
- kein impliziter globaler Zustand
- keine „kurzfristigen“ Architekturabkürzungen
- deterministische Verarbeitung
- expliziter Datenfluss statt versteckter Magie

---

## Red Flags (Sofortiger Stopp)

Wenn eine der folgenden Aussagen zutrifft, liegt ein Architekturfehler vor:

- eine Klasse greift auf mehrere Verantwortungsbereiche zu
- Multithreading würde das Design brechen
- globaler Zustand wird „zur Vereinfachung“ eingeführt
- eine Komponente weiß zu viel über andere Systeme

---

## Entscheidungsregeln

- explizite Schritte sind wichtiger als Magie
- Komplexität reduzieren, nicht verstecken
- Architektur steht immer über Code

---

## Build-Anleitung

Das Projekt verwendet **CMake** und setzt auf einen Out-of-Source-Build.

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

---

## Zukunft (Phase 2)

Phase 2 wird unter anderem folgende Bereiche einführen:

- editierbare Datenmodelle
- Editor-Logik
- UI-Systeme
- Save- und Compile-Workflows

Mit Beginn von Phase 2 wird diese README entsprechend angepasst.

---

## Abschließende Hinweise

Dieses Repository stellt **kein fertiges Produkt**, sondern ein **architektonisches Fundament** dar.

Es dient dazu:
- saubere Experimente zu ermöglichen
- Architekturentscheidungen festzuhalten
- eine stabile Basis für zukünftige Entwicklung zu schaffen

---
