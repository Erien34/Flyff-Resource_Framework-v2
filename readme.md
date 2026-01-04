# Flyff Overhaul – Vollständige kanonische Referenz

> **Hinweis:** Dieses Dokument ist eine vollständige Zusammenführung aller kanonischen Modelle.

> Es wurde **nichts gekürzt oder entfernt**.


---




---

# 📄 Quelle: `readme.md`


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



---

# 📄 Quelle: `world_model_goal_architecture.md`


# World Model – Zielarchitektur (Phase 1 & Ausblick Phase 2)

> **Zweck dieses Dokuments**
>
> Dieses Dokument hält die **finale Zielstruktur** der Welt-Modernisierung fest.
> Es dient als **Referenz**, damit Architektur‑, Tool‑ und Engine‑Entscheidungen konsistent bleiben.
> Änderungen an der Weltlogik müssen sich **an diesem Dokument messen lassen**.

---

## 1. Grundprinzipien

### 1.1 Kanonisches Weltmodell
- Die Welt existiert **einmal** in einem **kanonischen, engine‑neutralen Modell**.
- Dieses Modell ist:
  - unabhängig von FlyFF
  - unabhängig von Unity / Server / Runtime
  - versionierbar
  - validierbar

**FlyFF‑Daten sind Input, nicht Wahrheit.**

---

### 1.2 Trennung der Ebenen

```text
Legacy Daten (Client / Resource / Source)
        ↓
Tool Parser & Normalisierung
        ↓
Kanonisches World‑Modell (.world / .area)
        ↓
Adapter (Unity / Runtime / Server)
```

- Unity ist **Editor & Visualisierung**, nicht Daten‑Owner
- Server / Client nutzen **kompiliertes Runtime‑Modell**
- Nur das Tool darf das kanonische Modell verändern

---

## 2. Welt-Hierarchie (final)

### 2.1 WorldScene

> **WorldScene = globaler Kontext & Regelraum**

Eine WorldScene entspricht **genau einem World‑Ordner**.

Beispiele:
- Overworld
- Dungeon
- Arena
- Event‑World

**Aufgaben der WorldScene:**
- Welttyp definieren
- globale Regeln festlegen
- erlaubte / verbotene Zonen‑Typen bestimmen
- Lebenszyklus (persistent, instanziert, resetbar)
- Übergänge zu anderen WorldScenes


---

### 2.2 WorldArea

> **WorldArea = räumlicher Container**

- Eine WorldArea entspricht **in der Regel einer ehemaligen `.lnd` Datei**
- Eine WorldScene enthält **N WorldAreas**

**WorldArea definiert:**
- Terrain‑Referenz
- räumliche Grenzen
- optionale Area‑Tags (z. B. `boss_room`, `corridor`)
- **keine harte Gameplay‑Logik**

---

### 2.3 Zone (zentrale Logikeinheit)

> **Zonen sind die einzige Stelle, an der Verhalten definiert wird**

- Zonen liegen **innerhalb einer WorldArea**
- Es gibt **beliebig viele Zonen pro Area**
- Zonen können sich **überlappen**

**Eigenschaften einer Zone:**
- Typ (Semantik)
- Shape (Polygon, Kreis, Rechteck, etc.)
- Priorität
- Regel‑Set


---

## 3. Zonen‑Konzept (Schlüsselidee)

### 3.1 Zonen ersetzen Flags

❌ Alt:
```text
tileFlag = 0x24
```

✅ Neu:
```text
Zone { type = "no_walk" }
```

- Bedeutung ist explizit
- Verhalten ist vorhersagbar
- Kombination erzeugt Komplexität

---

### 3.2 Zonen sind universell

Zonen regeln **alles**, u. a.:
- Bewegung (NoWalk, NoFly)
- PvP / SafeZones
- Spawns
- Monster‑Verhalten
- Dungeon‑Progression
- Boss‑Phasen

**Neue Features = neue Zonen**, nicht neuer Code.

---

### 3.3 Shapes

Zonen sind **nicht grid‑ oder tile‑gebunden**.

Erlaubte Shapes:
- Polygon
- Kreis
- Rechteck
- Dreieck
- (optional) Spline / Freiform

---

### 3.4 Priorität & Überlagerung

- Mehrere Zonen können gleichzeitig gelten
- Höhere Priorität überschreibt niedrigere

Beispiel:
- PvP‑Zone
- darin SafeZone
→ SafeZone gewinnt

---

## 4. Bewegungs‑ & Begrenzungslogik

### 4.1 Kein Blockieren von Spielern – Blockieren von Bewegung

- Bewegung wird **vor der Ausführung geprüft**
- Nicht: „Spieler ist in verbotener Zone“
- Sondern: „Zielposition ist nicht erlaubt“

**Ergebnis:**
- keine unsichtbaren Wände
- keine Pixel‑Perfect‑Probleme
- keine Exploits

---

## 5. Dungeon‑ & Spezialverhalten

### 5.1 Dungeon ist kein Code‑Sonderfall

> **Dungeon = WorldScene mit bestimmten Regeln**

- Aggressive Monster
- kein Aggro‑Reset
- Teleport‑Verfolgung
- Progression über Zonen

---

### 5.2 Progression über Zonen

Beispiele:
- 50 Kills → Boss 1
- weitere 50 → Boss 2
- Boss‑Phasen über Zonen‑Regeln

**Dungeon‑Gameplay = Daten, nicht Skript‑Code**

---

## 6. Autorität & Overrides

- Zonen bleiben objektiv
- Autorität entscheidet, ob Regeln gelten

Beispiele:
- GM ignoriert NoWalk
- Admin deployt Hotfix

**Regeln werden nicht verändert – nur umgangen**

---

## 7. Dateiformate (Ziel)

### 7.1 `.world`
- speichert WorldScene
- eine Datei pro World

### 7.2 `.area`
- speichert WorldArea + Zonen
- eine Datei pro Area

### 7.3 `.lnd`
- reines Import‑Format
- wird nicht mehr produktiv genutzt

---

## 8. Unity‑Integration

- Unity ist **Adapter & Editor**
- Zonen werden visuell dargestellt (Gizmos / Overlays)
- Speichern erfolgt **immer zurück in `.world` / `.area`**

Unity ist **nicht** die Wahrheit.

---

## 9. Runtime‑Modell

- eigenes, kompaktes Runtime‑World‑Format
- keine Editor‑Daten
- schnelle Abfragen

```text
.world / .area → Compiler → RuntimeWorld.bin
```

---

## 10. Phase‑1‑Ziel (Zusammenfassung)

Nach Phase 1 existiert:
- ein vollständiges, engine‑neutrales World‑Modell
- klare Trennung von Struktur, Raum und Verhalten
- Grundlage für Editor, Runtime und Live‑Ops

---

## Leitsatz

> **WorldScene setzt den Rahmen.**  
> **WorldArea teilt den Raum.**  
> **Zonen definieren Verhalten.**  
> **Neue Features entstehen durch Kombination – nicht durch Code.**




---

# 📄 Quelle: `worldboss.md`


# Worldbosse & Weltbuffs – Kanonisches Modell

## 1. Ziel & Designphilosophie

Worldbosse sind **serverweite PvE-Kooperationsereignisse**.

Sie dienen **nicht** der direkten Charakter-Power, sondern:
- fördern Zusammenarbeit
- schaffen serverweite Erfolgsmomente
- aktivieren zeitlich begrenzte Weltbuffs
- liefern kosmetische Langzeitbelohnungen
- vermeiden Alt-Missbrauch und Pflicht-Content

**Grundsatz:**
> Worldbosse belohnen den Server – nicht den einzelnen Charakter.

---

## 2. Definition Worldboss

Ein Worldboss ist ein einzigartiges, zeitlich begrenztes PvE-Event mit folgenden Eigenschaften:

- fester Spawnpunkt (World Scene / Area)
- hoher Schwierigkeitsgrad
- serverweite Teilnahme möglich
- kein klassischer Loot-Drop
- eigener Lebenszyklus (Spawn → Aktiv → Erfolg / Fehlschlag)

---

## 3. Spawn & Lebenszyklus

### 3.1 Spawn-Zyklus

- Worldbosse spawnen in **festen Zeitintervallen**
  - Beispiel: alle **3 Stunden**
- Pro Zyklus existiert **maximal ein aktiver Worldboss**

---

### 3.2 Aktive Phase

- Nach dem Spawn beginnt die **aktive Phase**
- Dauer: **fixe Zeitspanne**
  - Beispiel: **60 Minuten**
- Während dieser Zeit kann der Boss angegriffen werden

---

### 3.3 Erfolgsfall (Boss besiegt)

Wird der Worldboss **innerhalb der aktiven Phase** besiegt:

1. Boss stirbt
2. Teilnehmer erhalten **kosmetische Belohnungen**
3. Ein **Weltbuff** wird aktiviert
4. Der Weltbuff gilt **serverweit**
5. Dauer des Weltbuffs: **begrenzt**
   - Beispiel: **90 Minuten**

---

### 3.4 Fehlschlagsfall (Boss nicht besiegt)

Wird der Worldboss **nicht innerhalb der aktiven Phase** besiegt:

- Boss **despawnt automatisch**
- ❌ kein Weltbuff
- ❌ keine Belohnungen
- ❌ keine Ersatz- oder Trostmechanik
- nächster Spawn erfolgt regulär

**Harte Regel:**
> Ohne erfolgreichen Kill existiert kein Weltbuff.

---

## 4. Teilnahme & Contribution

### 4.1 Teilnahmebedingung

- **Minimaler Schadensbeitrag genügt**
- Keine Ranglisten
- Kein DPS-Wettbewerb
- Kein Last-Hit-Prinzip

---

### 4.2 Anti-Exploit-Prinzip

Worldbosse geben **keine direkte Power**:

- keine Items mit Stats
- keine Set-Progression
- kein Crest-Fortschritt
- keine Cap-Extensions

➡️ Mehrere Charaktere oder Alts bringen **keinen Vorteil**.

---

## 5. Belohnungen

### 5.1 Teilnehmerbelohnungen (direkt)

Teilnehmer erhalten **ausschließlich kosmetische Inhalte**, z. B.:

- Waffen-, Rüstungs- oder Cloak-Skins
- Pet-Appearances
- Titel
- Emotes
- kosmetische Rezepte

**Explizit ausgeschlossen:**
- Power-Stats
- Rarity-Bonis
- Progressionswährungen

---

### 5.2 Serverweite Belohnung (indirekt)

- Erfolgreicher Kill aktiviert einen **Weltbuff**
- Weltbuff gilt **für alle Spieler**
- Teilnahme ist **nicht erforderlich**, um vom Buff zu profitieren

---

## 6. Weltbuff-Modell

### 6.1 Grundprinzip

- Weltbuffs sind **zeitlich begrenzt**
- Weltbuffs sind **serverweit aktiv**
- Weltbuffs sind **nicht stapelbar**
- Weltbuffs sind **leicht positiv**, nicht spielentscheidend

---

### 6.2 Buff-Auswahl

- Es existiert ein **Pool an Weltbuffs**
- Nach jedem erfolgreichen Worldboss-Kill wird **zufällig ein Buff gewählt**
- Der zuletzt aktive Buff ist **temporär ausgeschlossen**
  - derselbe Buff kann **nicht direkt erneut auftreten**

---

### 6.3 Beispiele für Weltbuffs

Erlaubt:
- +X % Material-Drop-Chance
- +X % Chance auf Dungeon-Entry-Fragmente
- +X % Monster-Spawnrate
- +X % Quest-Fortschritt
- +X % Aktivitätsbonus

Nicht erlaubt:
- direkter Schadensbonus
- Crit-Chance oder Crit-Damage
- Crest- oder Set-Progression
- PvP-Power

---

## 7. PvE / PvP-Trennung

- Weltbuffs wirken **nur im PvE**
- PvP ist **nicht betroffen**
- PvP-Balance bleibt vollständig isoliert

---

## 8. Persistenz

- Weltbuffs sind **nicht persistent**
- Server-Restart entfernt aktive Weltbuffs
- Worldboss-Status wird **nicht gespeichert**

---

## 9. Design-Zusammenfassung

- Worldbosse sind **freiwilliger Content**
- Kein Kill = kein Buff
- Kein Power-Creep
- Kein Alt-Missbrauch
- Kein Zwang zur Teilnahme
- Server-Erfolg steht über Individual-Erfolg

---

## 10. Beziehung zu anderen Modellen

| Modell | Beziehung |
|------|----------|
| World-Scene-Modell | Worldbosse sind an Areas gebunden |
| Quest-Modell | optionale Worldboss-Quests möglich |
| Item-Modell | ausschließlich kosmetische Items |
| PvP-Regelwerk | vollständig getrennt |
| Progressionsmodell | indirekt über Weltbuffs |

---

**Status:**  
Worldboss-Modell ist **final, kanonisch und versionsstabil**.



---

# 📄 Quelle: `Quest- & Progressionsmodell – Kanonisch.md`


# Quest- & Progressionsmodell – Kanonisch

## 1. Ziel & Designprinzipien

Das Quest- und Progressionssystem dient dazu

- Spieler strukturiert durch Inhalte zu führen
- langfristige Motivation zu erzeugen
- Aufholen (Catch-up) zu ermöglichen
- FOMO zu vermeiden
- harte Progressions-Gates zu kontrollieren
- PvE- und PvP-Inhalte klar zu trennen

Grundsatz
 Progression soll planbar, fair und nachholbar sein – nicht exklusiv oder verpflichtend.

---

## 2. Progressionsphasen

### 2.1 Levelphase (Early- & Midgame)

- Fokus Levelaufstieg, Einführung in Systeme
- Inhalte
  - Story-Quests
  - Zonen-Quests
  - Einfache Dungeon-Einstiege
- Ziel
  - Erreichen der Maximalstufe
  - Sammeln von Grundmaterialien
  - Vorbereitung auf Endgame

Nach Erreichen der Maximalstufe endet die klassische Levelprogression.

---

### 2.2 Endgame-Phase

- Fokus horizontale Progression
- Systeme
  - Sets
  - Crests
  - Capstones
  - Dungeons
  - Weltbosse
- Ziel
  - Build-Optimierung
  - Spezialisierung
  - Langzeitmotivation

---

## 3. Quest-Typen

### 3.1 Einmalige Quests

- Nur einmal pro Charakter abschließbar
- Typische Inhalte
  - Story-Fortschritt
  - System-Freischaltungen
  - Einführungsquests (Crests, Dungeons, PvP)
- Belohnungen
  - Items
  - Sets (Early-Endgame)
  - Rezepte
  - kosmetische Inhalte

---

### 3.2 Tägliche Quests

- Einmal pro Tag abschließbar
- Beispiele
  - Töte X Monster
  - Schließe einen Dungeon ab
  - Sammle Materialien
- Belohnungen
  - Gold
  - Materialien
  - Fragmente
- Geringere Belohnung als wöchentliche Quests

---

### 3.3 Wöchentliche Quests

- Einmal pro Woche abschließbar
- Beispiele
  - Besiege Dungeon-Bosse
  - Nimm an GvG teil
  - Schließe Gruppencontent ab
- Belohnungen
  - hochwertige Materialien
  - Dungeon-Entry-Fragmente
  - Fortschrittssysteme

---

### 3.4 Wiederholbare (unendliche) Quests

- Unbegrenzt abschließbar
- Geringe, skalierende Belohnungen
- Fokus
  - Farming
  - Grinding
  - Materialien
- Keine exklusiven Rewards

---

## 4. Quest-Limits & Kontrolle

- Maximalanzahl aktiver Quests pro Charakter
- Verhindert
  - Überforderung
  - UI-Spam
  - Exploits
- Limits sind systemweit konfigurierbar

---

## 5. Catch-up-Mechaniken

### 5.1 Zeitbasierte Freischaltung

Nach einer definierten Patch-Laufzeit

- tägliche & wöchentliche Questbelohnungen werden wiederholbar
- alternative Fortschrittswege werden freigeschaltet
- zusätzliche Dungeon-Entries können
  - erspielt
  - hergestellt
  - gehandelt werden

➡️ Spieler mit viel Zeit können gezielt aufholen.

---

### 5.2 Einstieg ins Endgame

Nach Erreichen der Maximalstufe

- Spieler können mit gesammelten Materialien
  - ein Einsteiger-Endgame-Set craften
- Einstiegshürde ist bewusst niedrig
- Ziel
  - sofort spielbarer Endgame-Zugang
  - kein Gear-Wall

---

## 6. Dungeon-Progression

- Dungeons sind primärer Power-Progression-Content
- Dungeon-Zugänge
  - begrenzt
  - über Quests, Fragmente oder Crafting erweiterbar
- Schwierigkeit & Belohnung skalieren

---

## 7. Welt- & Farmcontent

### 7.1 Farm-Maps

- Nach Maximalstufe verfügbar
- Inhalte
  - starke Monster
  - Giants
  - periodische Boss-Spawns
- Drops
  - handelbare Materialien
  - Crafting-Ressourcen

---

### 7.2 Weltbosse (Integration)

- Weltbosse sind kein Pflicht-Content
- Erfolgreicher Kill
  - aktiviert Weltbuffs
  - liefert kosmetische Belohnungen
- Kein Kill
  - keine Belohnung
  - kein Buff

(Worldboss-Modell siehe separates Dokument)

---

## 8. PvE- & PvP-Trennung

- Quest-Progression ist primär PvE-basiert
- PvP-Quests existieren separat
- PvE-Fortschritt gibt keine PvP-Dominanz
- PvP hat eigene Regeln, Sets und Progression

---

## 9. Progressions-Gates & Capstones

- Capstones sind Meilensteine
  - alle X Level
  - oder über spezielle Quests
- Capstones
  - erweitern Caps
  - schalten neue Systeme frei
- Kein Item kann Capstones ersetzen

---

## 10. Design-Zusammenfassung

- Kein Pflicht-Content
- Kein FOMO-Zwang
- Aufholen ist möglich
- Progression ist horizontal + vertikal
- Schwierigkeit ↔ Belohnung sind gekoppelt
- Community-Content wird belohnt, nicht erzwungen

---

## 11. Beziehung zu anderen Modellen

 Modell  Beziehung 
----------------
 Item-Modell  Quest-Rewards & Set-Zugänge 
 Worldboss-Modell  optionale Integration 
 PvP-Regelwerk  getrennte PvP-Quests 
 Economy-Modell  Material- & Währungsflüsse 
 World-Modell  Quests sind zonen- & area-gebunden 

---

Status  
Quest- & Progressionsmodell ist final, kanonisch und versionsstabil.



---

# 📄 Quelle: `Economy- & Crafting-Modell – Kanonisch.md`


# Economy- & Crafting-Modell – Kanonisch (Final)

## 1. Ziel & Leitprinzipien

Dieses Modell stellt eine langfristig stabile, faire und patch-saubere Wirtschaft sicher.

Designziele:
- Kein Reparatursystem
- Crafting als primärer Goldsink
- Keine Power durch Echtgeld
- Kein Stockpiling-Vorteil über Patches hinweg
- Klare Trennung von PvE / PvP
- Aufholen (Catch-up) über Effizienz, nicht über Boosts

**Grundsatz:**
> Materialien und Rezepte sind an Content-Tiers gebunden – nicht zeitlos.

---

## 2. Währungen

### 2.1 Primäre Währung

**Gold**
- universelle Handels- und Dienstleistungswährung
- Hauptverwendung:
  - Crafting
  - Auktionshaus / Marktgebühren
  - Schutzrollen
  - Re-Roll-Rollen
  - NPC-Dienste (Fusion, Enhancement, Services)

**Gold ist der zentrale Sink der Economy.**

❌ Reparaturkosten existieren nicht.

---

### 2.2 Sekundäre Währungen (handelbar, aber ineffizient)

- Dungeon-Entry-Fragmente
- Event-Währungen
- Capstone-Materialien
- Crest-Fragmente

Eigenschaften:
- handelbar
- geringe Gold-Effizienz beim Verkauf
- Design-Intent: **Selbst nutzen ist immer besser als verkaufen**
- Verkauf dient nur als Ausweichoption

---

## 3. Materialien

### 3.1 Material-Tiers (pro Content-Patch)

Jeder Major-Content-Patch besitzt **eigene Materialsets**.

#### Tier-Struktur:
1. **Basismaterialien**
   - Drops von normalen Monstern
2. **Verbesserte Materialien**
   - Giants
   - Dungeon-Monster
3. **Hochwertige Materialien**
   - Dungeon-Bosse
4. **Spezialmaterialien**
   - Capstone-Dungeons
   - Klassenabschluss
   - System-Freischaltungen

Zusätzlich:
- tägliche / wöchentliche Questmaterialien

---

### 3.2 Patch-Gebundene Materialzyklen

- Materialien sind **nicht patchübergreifend relevant**
- Patch 1.0 → Material-Set A  
- Patch 2.0 → Material-Set B  
- Patch 3.0 → Material-Set C  

Alte Materialien:
- bleiben im alten Content
- verlieren Power-Relevanz
- werden **nicht** für neuen Progress verwendet

➡️ Kein Stockpiling-Vorteil.

---

## 4. Materialfluss

- Farming-Areas → Materialien
- Crafting → Materialverbrauch
- Recycling → Teilrückgewinnung (nie 100 %)
- Markt → Umverteilung

**Kein Material ist nutzlos.**

---

## 5. Crafting-System

### 5.1 Crafting-EXP & Level

- Jedes Crafting-Rezept gibt **Crafting-Experience**
- Crafting-Level steigen durch Nutzung
- Rezepte werden **automatisch über Crafting-Level freigeschaltet**
- Keine Rezepte aus:
  - Quests
  - Dungeons
  - Worldbossen

➡️ Crafting ist ein eigenes Progressionssystem.

---

### 5.2 Crafting-Ziele

Crafting erlaubt:
- Endgame-Einstiegssets
- Verbrauchsgegenstände
- Scrolls (Schutz, Re-Roll)
- Dungeon-Entries
- Item-Verbesserungen
- Vorbereitung für:
  - Rarity-System
  - Crest-System
  - Quest-Progression

---

### 5.3 Rezept-Effizienz über Patches

- Neue Patches bringen:
  - effizientere Rezepte
  - neue Materialien
- Gleicher Output bei geringerem Zeit- & Materialaufwand
- Alte Rezepte bleiben funktional, aber ineffizient

➡️ Catch-up durch Effizienz, nicht durch Zwang.

---

## 6. Item-Verbesserung (Enhancement)

- Items können verstärkt werden (+X)
- steigende Stufen → steigendes Fehlschlagsrisiko
- Verbrauch:
  - Gold
  - Materialien
- Schutzrollen verhindern Item-Verlust
- Enhancement erfolgt bei speziellen NPCs

---

## 7. Rarity-System (Integration)

### 7.1 Rarity-Stufen
- Normal
- Selten
- Episch
- Legendär

### 7.2 Rarity-Fusion
- Nur gleiche Rarity fusionierbar
- Verbrauch:
  - Gold
  - Materialien
- Fehlschlag möglich
- Schutzrollen reduzieren Risiko

---

## 8. Recycling & Zerlegung

- Alle Items können zerlegt werden
- Rückgewinnung:
  - Materialfragmente
  - Rarity-Staub
- Rückgewinnung ist **teilweise**, nie vollständig

➡️ Bad-Luck-Protection ohne Power-Exploit.

---

## 9. Scrolls & Rollen

### 9.1 Schutzrollen
- verhindern Zerstörung bei:
  - Rarity-Fusion
  - Enhancement
  - Crest-Kombination
- craftbar
- handelbar
- nicht Echtgeld-exklusiv

### 9.2 Re-Roll-Rollen
- Stats neu würfeln
- Gold- & Materialkosten
- wichtiger Goldsink

---

## 10. PvE / PvP-Trennung

- PvE- & PvP-Sets haben:
  - getrennte Materialien
  - getrennte Crafting-Pfade
- Keine direkte Konvertierung von:
  - PvE-Power → PvP-Power
- Rarity kann in beiden Systemen existieren, aber getrennt

---

## 11. Echtgeld & Monetarisierung

Erlaubt:
- Kosmetik (aufwendig, visuell hochwertig)
- Komfort
- Premium / QoL
- Offline-Vendors (Bequemlichkeit)

Verboten:
- Stats
- Power
- Progression
- XP / Drop-Vorteile
- AFK-Vorteile

Premium:
- kein Schutz vor Anti-AFK
- keine Progressionserhöhung

---

## 12. Anti-Inflationsmaßnahmen

- Crafting-Kosten (primärer Sink)
- Marktgebühren
- Schutzrollenverbrauch
- Re-Roll-Kosten
- NPC-Dienste

❌ Keine Reparaturkosten.

---

## 13. Design-Zusammenfassung

- Crafting ist der Kern der Economy
- Materialien sind Content-gebunden
- Kein Stockpile-Meta
- Kein Zwang zu Altkontent
- Fairer Catch-up
- Langfristig stabile Wirtschaft

---

## 14. Beziehung zu anderen Modellen

| Modell | Beziehung |
|------|----------|
| Quest-Modell | Material- & Entry-Flows |
| Item-Modell | Rarity, Enhancement |
| Crest-Modell | Material- & Scrollverbrauch |
| Worldboss-Modell | kosmetische Belohnungen |
| PvP-Regelwerk | getrennte Economy |

---

**Status:**  
Economy- & Crafting-Modell ist **final, kanonisch und versionsstabil**.



---

# 📄 Quelle: `item_data_model.md`


# Canonical Item Data Model

## Purpose
This document defines the canonical item system. It establishes item categories, equip layers, progression systems, PvE/PvP separation rules, and long-term scalability constraints. The item model is engine-agnostic and serves as the authoritative reference for tools, runtime, and exports.

---

## Core Principles

1. **Clear Separation of Concerns**
   - Power, utility, cosmetics, and economy are strictly separated.
   - No item serves multiple conflicting roles.

2. **Deterministic Progression with Controlled RNG**
   - RNG exists but is bounded, controllable, and reversible via systems (scrolls, time).

3. **PvE / PvP Isolation**
   - PvE items must not dominate PvP.
   - PvP progression must not be required for PvE.

4. **Long-Term Scalability**
   - Caps, diminishing returns, and extension mechanisms are explicitly defined.

---

## Item Categories

### A. Gameplay Items (Power)
- Gear (Weapons, Armor, Shield, Jewelry)
- Sets (PvE)
- PvP Gear & PvP Sets
- Crests
- Cloaks
- Power Pets

### B. Cosmetic Items (Appearance Only)
- Skins (Transmog)
- Cashshop Cosmetics
- Event Cosmetics

### C. Progression & System Items
- Crest Splinters
- Capstone Unlock Items
- Scrolls (Reroll, Protection)
- Pet Eggs
- Pet Reroll Scrolls

### D. Economy & Utility Items
- Crafting Materials
- Recycling Outputs
- Quest Items (Non-equipable)

---

## Equip & Inventory Layers

### Layer 1: Gameplay Equip
- Weapons
- Armor
- Shield
- Jewelry
- Cloak **OR** Crest (exclusive slot)
- Power Pet (1 active)

### Layer 2: Cosmetic Overlay
- Skins
- Transmog Overrides

### Layer 3: PvP Equip
- PvP Gear
- PvP Sets
- Lightning Crest (PvP only)

### Layer 4: Inventory (Non-equipable)
- Materials
- Splinters
- Scrolls
- Quest Items

---

## Gear & Sets

### Normal Gear
- Base Stats
- Secondary Stats
- Rarity-enabled
- No special mechanics

### Sets (PvE)
- Early/Midgame power focus
- Fixed piece counts
- Deterministic bonuses
- No cap extensions
- PvP effects disabled or normalized

### PvP Sets
- PvP-only activation
- Flat, controlled bonuses
- No burst or proc mechanics

---

## Crests

### Crest Types
- Fire (STR)
- Water (INT)
- Earth (STA)
- Wind (WIL)
- Lightning (PvP only)
- Colorless (Hybrid)

### Crest Rules
- Share slot with Cloak
- Provide scaling progression
- Unlock Capstones
- Allow cap extensions (PvE only)
- PvP: only Lightning Crest is active

---

## Rarity System

### Rarity Levels
- Normal → Uncommon → Rare → Epic → Legendary

### Rarity Properties
- Adds linear stats only
- No set bonuses
- No cap extensions
- PvP-compatible (optionally normalized)

### Fusion Rules
- Only same rarity items can fuse
- Fusion may fail
- Failure may destroy one or both items

### Recycling
- All gear can be dismantled
- Outputs crafting resources
- Supports bad-luck protection

---

## Pet System

### Pet Types

#### Utility Pets
- Auto-loot functionality
- Cosmetic skins allowed
- No stats
- No PvP interaction

#### Power Pets
- One active per character
- PvE only
- No skins
- Tradeable
- Destroyable, not recyclable
- No rarity, no set affiliation

### Power Pet Stat Structure
- 1 Base Stat (determines pet archetype)
- 2 Secondary Stats
- 2–3 Tertiary Stats

#### Base Stat
- STR / DEX / INT / STA / WIL
- Fixed type
- Roll range (min–max)

#### Secondary Stats
- Derived from pet archetype
- Rollable within small ranges
- Cannot duplicate

#### Tertiary Stats (Pet-exclusive)
- PvE Damage Bonus
- Boss / Elite Damage
- PvE Damage Reduction
- HP Leech (capped)

### Pet Progression
- Pet hatches from egg after time
- Passive leveling based on online time
- Stat slots unlock over time

### Pet Customization
- Scroll-based stat rerolls
- Separate scrolls for type vs value
- Increasing reroll cost over time

### Pet Quality Display
- Visual indicators for high/max rolls
- No gameplay impact

---

## PvP Rules

- PvE Sets: disabled or normalized
- PvE Crests: disabled
- Power Pets: disabled
- Utility Pets: disabled
- Rarity: active (optionally scaled)

---

## Trade & Binding

- Utility Pets: tradeable
- Power Pets: tradeable
- Crests: progression-bound
- Scrolls: tradeable
- Quest Items: bound

---

## Summary

The item system is modular, deterministic, and scalable.
Each system has a clear role:
- Sets provide stability
- Crests provide long-term scaling
- Rarity provides linear optimization
- Pets provide PvE meta depth
- PvP remains fair and isolated

This model is the authoritative reference for all item-related systems.




---

# 📄 Quelle: `gameplay_data_model.md`


# Gameplay-Datenmodell (kanonisch)

## 1. Ziel
Dieses Dokument definiert das **kanonische Gameplay-Datenmodell**. Es ist engine-neutral, tool-zentriert und dient als verbindliche Referenz für Server, Client, Tools und Editoren.

Ziele:
- klare Trennung von Primär-, Sekundär- und Runtime-Logik
- exploit-resistent
- patch- und content-skalierbar
- vollständig datengetrieben

---

## 2. Primärstats
Primärstats sind permanente Charakterattribute. Ihre Wirkung ist **klassenabhängig interpretiert**, nicht global identisch.

### STR – Strength
- wirkt ausschließlich auf **physische Attacks & physische Skills**
- skaliert **Physical Attack (ATK)**
- kein Einfluss auf HP, DEF oder Speed

### DEX – Dexterity
- skaliert **Attack Speed** (prozentual)
- skaliert **Ranged Damage**
- kein Einfluss auf Magie oder Defensive

### INT – Intelligence
- skaliert **Magic Attack (M-ATK)**
- erhöht **Mana Pool**
- skaliert **magische Skills**

### STA – Stamina
- erhöht **maximale HP**
- erhöht **DEF (Basis-Mitigation)**

### WIL – Willpower
- erhöht **Buff-Dauer** (nicht Stärke)
- erhöht **Heilungseffektivität**
- reduziert **Ressourcenkosten** (Mana/FP)
- alle Effekte unterliegen Caps & Diminishing Returns
- kein Einfluss auf Cooldowns

---

## 3. Secondary Stats (sichtbar)
Secondary Stats sind abgeleitete Werte und für Spieler sichtbar, um Item- und Build-Entscheidungen zu ermöglichen.

- ATK (Physical Attack)
- M-ATK (Magic Attack)
- DEF
- Max HP
- Attack Speed (%)
- Movement Speed (%)
- Mana Pool
- FP / SP Pool
- Heal Power
- Mana Cost Multiplier
- Damage Range (Min–Max)
- Crit Chance
- Crit Damage
- Block Chance

---

## 4. Attack Speed Modell
- jede Waffe besitzt einen **internen Weapon Speed** (nicht sichtbar)
- Attack Speed ist ein **prozentualer Modifier (0–100%)**
- 100% = eine vollständige Animation erzeugt einen Hit
- <100% = längere Zeit bis zum Hit
- >100% ist **nicht erlaubt**

**Hard Cap:** 100%

---

## 5. Crit-System

### Gültigkeit
- Crits gelten **nur für physische Auto-Attacks & physische Skills**
- Magische Angriffe und Heals können nicht critten

### Crit Chance
- sichtbarer Wert
- Cap ist **content-/patchabhängig**
- Diminishing Returns vor Cap

### Crit Damage
- Multiplikator auf Schaden bei Crit
- ebenfalls content-abhängiger Cap
- wirkt nur bei erfolgreichen Crits

---

## 6. Cost Reduction (WIL)

### Grundregel
- ohne WIL-Investment: 100% Skillkosten

### Verdopplungsmodell (Beispiel Level 1)
- Basis-WIL: 15 → 100% Kosten
- 30 WIL → 75% Kosten
- 60 WIL → 50% Kosten (**Hard Cap**)

### Nach Erreichen des Cost-Caps
- keine weitere Kostenreduktion
- Buff-Dauer-Bonus wird **verdoppelt**

---

## 7. Heilungseffektivität
- skaliert unabhängig von Cost Reduction
- eigene Caps & Diminishing Returns
- zusätzliche Heilwirkung nur bei **hohem Schadensdruck**

Heilung reagiert auf:
- fehlende HP
- eingehenden Schaden

Ziel:
- Support ist in kritischen Momenten stark
- Tank bleibt auf Consumables & Mitigation angewiesen

---

## 8. Block-System

### Block Chance
- nur für physische Angriffe relevant
- Base-Werte z. B. durch Schilde
- **Hard Cap: 75%**

### Overcap
- erhöht **Block Damage Reduction** (nicht Chance)
- Maximalreduktion: **99%**

Block negiert keinen Hit vollständig.

---

## 9. Damage Intake Pipeline (final)

1. Basis-Damage Range des Angreifers
2. Monster-Kategorie-Koeffizient (Normal / Giant / Boss)
3. Block Check (nur physisch)
   - bei Block: Block Damage Reduction
4. DEF & Basis-Mitigation (STA + Gear, Level-Scaling)
5. Element Suit Modifier (defensiv)
6. Self-Buffs / Damage-Glättung
   - Templer: Self-Mitigation
   - Seraph: Gruppen-Mitigation
7. Crit Check (nur physisch & ungeblockt)
8. Final Damage

---

## 10. Rollenlogik

### Templer
- primärer Dungeon-Tank
- kontrolliert Damage-Spikes über aktive Skills
- Block, DEF und HP entscheidend

### Seraph
- Gruppen-Support
- Heilung, Mitigation, Debuff-Kontrolle
- kein Burst-Damage-Fokus

---

## 11. Designprinzipien
- kein Stat ist allein ausreichend
- aktive Skillnutzung wichtiger als passive Werte
- klare Caps verhindern Exploits
- Systeme sind datengetrieben & erweiterbar

---

**Dieses Dokument ist die verbindliche Referenz für alle Gameplay-bezogenen Systeme.**




---

# 📄 Quelle: `canonical_asset_model.md`


# Kanonisches Asset‑Modell – Zielarchitektur (Phase 1)

> **Zweck dieses Dokuments**  
> Dieses Dokument definiert das **kanonische Asset‑Modell** des Projekts.  
> Es ist die **verbindliche Referenz** dafür, wie Assets intern verstanden, strukturiert und normalisiert werden – **unabhängig von FlyFF, Unity oder einer konkreten Engine**.

---

## 1. Grundprinzipien

### 1.1 Assets sind Inhalte, keine Dateien

- Dateien (`.o3d`, `.dds`, `.ani`, `.sfx`, …) sind **nur Input‑Artefakte**.
- Ein Asset ist eine **logische Einheit mit Bedeutung**.
- Ordnerstrukturen, Dateiendungen und Legacy‑Konventionen spielen **keine Rolle** im kanonischen Modell.

**FlyFF‑Assets sind Quellen, nicht Wahrheit.**

---

### 1.2 Trennung von Ebenen

```text
Legacy Dateien (Client / Resource)
        ↓
Parser & Strukturierung
        ↓
Kanonisches Asset‑Modell
        ↓
Adapter / Exporte / Runtime‑Builds
```

- Das kanonische Modell ist:
  - engine‑neutral
  - tool‑neutral
  - stabil & versionierbar
- Externe Tools (Unity, Blender, Photoshop …) sind **Editoren**, nicht Daten‑Owner.

---

## 2. Primitive Asset‑Bausteine (atomar)

Primitive Assets sind die **kleinsten bedeutungsvollen Einheiten**.  
Sie enthalten **keine Gameplay‑Logik** und **keine Engine‑Logik**.

### 2.1 Liste der primitiven Asset‑Typen (final)

```text
Texture
Sprite
Sound
Mesh
Skeleton
Animation
ParticleDefinition
```

---

### 2.2 Bedeutung der primitiven Typen

#### Texture
- rohe Bilddaten
- Nutzung: Materialien, Partikel, Masken, Effekte

#### Sprite
- 2D‑Visual mit semantischer Bedeutung
- kann genutzt werden als:
  - **Icon** (Items, Skills, Buffs)
  - **UI‑Visual** (Buttons, Frames, Panels)
- kann Zustände enthalten:
  - normal / hover / pressed / disabled

> **Icon und UI‑Sprite sind kein eigener Typ, sondern Nutzungskontext eines Sprites.**

#### Sound
- Audiodaten (WAV / OGG / etc.)
- keine Engine‑Bindung

#### Mesh
- Geometrie
- keine Materialien, keine Texturen

#### Skeleton
- Knochenstruktur
- Referenz für Animationen

#### Animation
- Animationsdaten
- referenziert ein Skeleton

#### ParticleDefinition
- Partikelbeschreibung
- rein visuell (keine Gameplay‑Logik)

---

## 3. Zusammengesetzte (logische) Assets

Zusammengesetzte Assets enthalten **keine eigenen Rohdaten**, sondern **Referenzen auf primitive Assets**.

Sie definieren **Bedeutung und Nutzung**, nicht Format.

---

### 3.1 ModelAsset

Verwendung:
- Charaktere
- Monster
- NPCs
- Items (3D)

```text
ModelAsset
 ├─ MeshRef
 ├─ SkeletonRef
 ├─ AnimationRefs[]
 ├─ TextureRefs[]
 └─ MaterialDefinition
```

- Texturen & Animationen werden **beim Export eingebettet**
- Intern bleiben sie **separat referenziert**

---

### 3.2 CharacterAsset

```text
CharacterAsset
 ├─ BaseModel (ModelAsset)
 ├─ Variants
 │   ├─ Gender
 │   ├─ Face
 │   ├─ Hair
 │   └─ Attachments
```

- ein Charakter = **ein Asset**
- Varianten sind Daten, keine eigenen Assets

---

### 3.3 ItemVisualAsset

```text
ItemVisualAsset
 ├─ ModelAsset
 └─ IconSprite
```

- Gameplay‑Item referenziert **nur dieses Asset**

---

### 3.4 SkillEffectAsset

```text
SkillEffectAsset
 ├─ VisualComponent
 │   ├─ ParticleRefs[]
 │   └─ AnimationRefs[]
 ├─ AudioComponent
 │   └─ SoundRefs[]
 └─ Timeline / Events
```

- vereint Partikel, Sound & Animation logisch
- Export als **ein Effekt‑Bundle**

---

## 4. Intern vs. Extern (bewusst getrennt)

### 4.1 Intern (kanonisch)
- maximale Trennung
- saubere Semantik
- perfekt für Analyse, Batch‑Editing, Live‑Ops

### 4.2 Extern (Adapter / Editor / Runtime)
- physisch gebündelt
- format‑spezifisch (GLB, PNG, WAV)
- niemals die Wahrheit

---

## 5. Exporte & Adapter

### 5.1 Modell‑Exporte
- GLB (Mesh + Texturen + Animationen embedded)
- für Blender, Unity, Runtime

### 5.2 Sprite‑ & Icon‑Exporte
- PNG
- evtl. Atlasing auf Adapter‑Ebene

### 5.3 Audio
- direkte Nutzung universeller Formate
- kein eigener Sound‑Converter

---

## 6. Runtime‑Ziel

> **Runtime muss einfach sein.**

Runtime sieht nur:

```text
RuntimeModelAsset
RuntimeSkillEffect
RuntimeSprite
```

- keine Cross‑File‑Lookups
- keine Sonderfälle
- keine Legacy‑Artefakte

---

## 7. Phase‑1‑Ziel (Assets)

Nach Phase 1 existiert:
- ein vollständig normalisiertes Asset‑Modell
- klare Trennung von Bedeutung & Format
- Basis für:
  - Batch‑Tools
  - Live‑Ops / Hotfixes
  - neue Engine / neue Runtime

---

## Leitsätze

> **Assets sind Inhalte, keine Dateien.**  
> **Intern trennen wir nach Bedeutung.**  
> **Extern bündeln wir nach Nutzung.**




---

# 📄 Quelle: `canonical_runtime_model.md`


# Kanonisches Runtime-Modell

> **Zweck dieses Dokuments**  
> Dieses Dokument beschreibt das **kanonische Runtime-Modell** des Systems.  
> Es definiert klar und verbindlich:
> - welche Runtime-Objekte existieren
> - wie Client und Server interagieren
> - was repliziert wird
> - was persistent ist und was nicht
> - wie Sicherheit, Anti-Cheat und Stabilität gewährleistet werden

Dieses Modell ist **engine-agnostisch**, **client-autoritätsfrei** und bildet die Grundlage für Server-, Client- und Tooling-Implementierungen.

---

## 1. Grundprinzipien

### 1.1 Server-Autorität

- Der **Server ist die einzige Quelle der Wahrheit**
- Der Client besitzt **keine Autorität über Spielzustände**
- Der Client sendet ausschließlich **Requests (Commands & Intents)**
- Alle Berechnungen, Validierungen und Zustandsänderungen erfolgen serverseitig

> **Der Client beschreibt Absicht – der Server beschreibt Realität.**

---

### 1.2 Trennung der Ebenen

| Ebene | Verantwortung |
|-----|---------------|
| Persistenz | Besitz, Fortschritt, Historie |
| Runtime-State | Momentaner Zustand, Simulation |
| Replikation | Sichtbare Ergebnisse |

---

## 2. Runtime-Objekte

### 2.1 RuntimeSession

**Lebensdauer:** Verbindung

Enthält:
- Session-ID
- Account-ID
- Character-ID
- Permissions (Player / GM / Admin)
- Rate-Limits
- Bot-Score (Session-lokal)

> RuntimeSessions werden **niemals persistiert**.

---

### 2.2 RuntimeWorld

**Lebensdauer:** Server-Laufzeit

- Aktive WorldScenes
- Globale Events
- Serverzeit

> RuntimeWorld enthält keine Assets, keine UI-Daten und keine Logikdefinitionen.

---

### 2.3 RuntimeScene

- Entspricht einer WorldScene
- Enthält aktive RuntimeAreas
- Steuert Szenenübergänge

---

### 2.4 RuntimeArea

- Entspricht einer WorldArea
- Kleinste Simulationseinheit
- Enthält:
  - RuntimeEntities
  - RuntimeZones
  - Trigger

---

### 2.5 RuntimeZone

- Logische Regelcontainer
- Beispiele:
  - NoWalkZone
  - NoFlyZone
  - PvPZone
  - SpawnZone

> Zonenlogik wird **niemals** an den Client repliziert.

---

### 2.6 RuntimeEntity

- Reine Logikrepräsentation
- Beispiele:
  - Spieler
  - Monster
  - NPCs
  - Projektile

Enthält:
- Entity-ID
- Position
- Rotation
- Status

---

### 2.7 RuntimePlayer

Spezialisierung von RuntimeEntity:
- Referenz auf RuntimeSession
- RuntimeInventory
- Skill-Cooldowns
- Input-States

---

### 2.8 RuntimeInventory

- Server-autoritativer Besitz zur Laufzeit
- Abbild der persistenten Inventardaten

---

## 3. Runtime Commands

> **Runtime Commands sind diskrete, einmalige Client-Anfragen.**

### 3.1 Command-Kategorien

- Combat & Skills
- Inventory & Equipment
- Interaction & World Objects
- Quests & Progression
- Trade, Shops & Economy
- Social & Communication
- System & Session

### 3.2 Eigenschaften

- Diskret
- Atomar
- Vollständig validierbar
- Können erfolgreich sein oder fehlschlagen

> Commands beschreiben **was passieren soll**, nicht **wie es passiert**.

---

## 4. Movement & Flight Intents

> **Intents beschreiben anhaltende Zustände, keine Aktionen.**

### 4.1 MovementIntent

- StartMovement
- ChangeDirection
- StopMovement
- Autorun

Server:
- prüft Zonen, Status, Kollisionsregeln
- simuliert Bewegung kontinuierlich

---

### 4.2 FlightIntent

- StartFlight
- ChangeFlightDirection
- StopFlight

Server:
- prüft No-Fly-Zonen
- limitiert Geschwindigkeit & Höhe

---

### 4.3 FollowIntent

- Movement-/Flight-Intent mit Target-Referenz
- Server berechnet Verfolgung

Abbruchbedingungen:
- Target verloren
- Zonenwechsel
- maximale Distanz überschritten
- expliziter StopIntent

---

## 5. Replikationsregeln

### 5.1 Sichtbarkeitsstufen

| Stufe | Beschreibung |
|-----|--------------|
| Owner | Vollständig sichtbar |
| Observer | Eingeschränkt sichtbar |
| Server | Verborgener Zustand |

---

### 5.2 Repliziert an alle (Public State)

- Positionen
- Animation-States
- sichtbare Effekte
- Tod / Alive

---

### 5.3 Owner-only State

- exakte HP / MP
- Cooldowns
- Buff-/Debuff-Dauern

---

### 5.4 Eingeschränkt sichtbar (Observer)

- HP als Balken (ohne Zahlen)
- Buffs / Debuffs:
  - limitierte Anzahl (z. B. N kürzeste Restzeiten)
  - nur Icons / Kategorien

---

### 5.5 Niemals repliziert

- Zonenparameter
- Kollisionsdaten
- Aggro / Threat
- Drop-Raten
- Random-Seeds
- Bot-Score

---

## 6. Persistenz vs. Runtime-State

### 6.1 Persistente Daten

- Charakterdaten (Level, Stats, Skills)
- Inventar & Besitz
- Quests & Fortschritt
- Gilden- & Bankdaten

---

### 6.2 Runtime-State (nicht persistent)

- HP / MP
- Buffs / Debuffs
- Cooldowns
- Positionen
- Combat-Zustände

---

### 6.3 Dungeon-Fortschritt (hybrid)

- Instanzgebundene Persistenz
- Persistiert:
  - Dungeon-Instanz-ID
  - Fortschrittsmarker (z. B. getötete Monster)
  - Boss-Flags
  - Entry-Safepoint

- Nicht persistiert:
  - Spielerposition im Dungeon
  - Kampfzustände

Disconnect-Verhalten:
- Rejoin am Entry-Safepoint
- Fortschritt bleibt erhalten

---

## 7. Anti-Cheat & Bot-Resistenz

- Server-seitige Zeit
- Varianz in Cooldowns & Spawns
- Verhaltensanalyse über Zeit
- Soft-Penalties vor Eskalation
- GM-/Admin-Assistenz statt Auto-Banns

> Sicherheit entsteht durch Architektur, nicht durch Client-Kontrolle.

---

## 8. Leitsätze

- **Der Client kennt Ergebnisse, nicht Regeln.**
- **Alles Entscheidende bleibt serverintern.**
- **Persistenz ist Besitz, Runtime ist Zustand.**
- **Unschärfe ist ein Feature.**

---

**Status:** Final – Grundlage für Implementierung & Tooling




---

# 📄 Quelle: `pvp_rule_set.md`


# Canonical PvP Rule Set

## Purpose
This document defines the canonical PvP rule set. PvP is treated as a **separate gameplay environment** with its own rules, progression, balance constraints, and data activation logic. PvP is explicitly designed to be **fair, skill-driven, and decoupled from PvE progression**.

---

## Core Principles

1. **Strict PvE / PvP Separation**
   - PvE progression must not dominate PvP.
   - PvP progression must not be required for PvE.
   - A new max-level PvP player must be competitive immediately.

2. **Skill Over Grind**
   - Player decision-making, positioning, and timing matter more than accumulated PvE power.

3. **Controlled Progression**
   - PvP progression is capped, gated, and non-exploitable.
   - No infinite farming loops.

4. **Fair Re-Entry**
   - Returning or new players are never locked out of PvP viability.

---

## PvP Environment Activation

When a player enters a PvP-enabled context (Arena, Duel, GvG):

```text
OnEnterPvP:
  Apply PvP Rule Set
  Disable PvE Crests
  Switch to PvP Equipment Rules
  Apply PvP Stat Scaling
```

When leaving PvP:

```text
OnExitPvP:
  Restore PvE Rule Set
```

---

## Crest Rules

### PvE Crests
- Fire / Water / Earth / Wind / Colorless Crests:
  - **Fully disabled in PvP**
  - No stat contribution
  - No cap extensions

### PvP Crest (Lightning Crest)
- Only Crest active in PvP
- Effects apply **only against players**
- Never affects PvE

Allowed effects (examples):
- Increased damage vs players
- Reduced damage taken from players
- Minor CC resistance

Restrictions:
- No PvE scaling
- No cap extensions
- Hard progression caps

---

## Equipment Rules

### PvE Equipment
- PvE Set bonuses:
  - Disabled or heavily normalized in PvP
- PvE Crest interactions:
  - Disabled

### PvP Equipment
- PvP Sets:
  - Designed specifically for PvP
  - Balanced, flat bonuses
  - No burst-oriented mechanics

### Cloak vs Crest Slot
- PvP:
  - Cloak and Lightning Crest are balanced to be equivalent choices
- PvE:
  - Crests dominate at endgame

---

## Rarity Rules

- Rarity bonuses:
  - **Remain active in PvP**
  - Apply only linear stat increases
  - No special mechanics

Optional normalization:
- Rarity effects may be scaled (e.g. 70–80%) for PvP stability

Rarity:
- Does not grant cap extensions
- Does not override PvP balance rules

---

## Stat Scaling & Caps

- Separate PvP caps apply
- PvE caps are ignored in PvP
- Caps are visible and communicated

Key examples:
- Crit Chance: PvP-specific cap
- Crit Damage: PvP-specific cap
- Movement Speed: hard-capped

---

## PvP Progression

### Sources
- Arena (primary)
- Guild vs Guild (primary)
- Duels (limited, non-farmable)

### Rewards
- PvP Crest Splinters (Lightning only)
- PvP cosmetics
- Titles
- Rankings

### Anti-Farming Rules

1. Kill Decay
   - Repeated kills on same player rapidly lose value

2. Account Pair Limits
   - PvP rewards limited per account pairing

3. Time & Activity Caps
   - Daily / weekly progression limits

---

## PvP vs PvE Power Parity

Design Goal:
- A top PvP player and a top PvE player have **comparable character power**
- Achieved through different systems

PvP excels in:
- Player combat

PvE excels in:
- Dungeon efficiency

Neither dominates the other.

---

## Summary

PvP is a fully self-contained environment:
- Separate rules
- Separate progression
- Separate balance

PvP rewards skill, not grind.
PvE rewards investment, not PvP.

Both coexist without invalidating each other.

