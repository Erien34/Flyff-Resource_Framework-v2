#pragma once

namespace core
{

enum class JobState
{
    Idle,       // Pipeline noch nicht gestartet
    Running,    // Pipeline arbeitet
    Done,       // Pipeline abgeschlossen
    Error       // Pipeline abgebrochen
};

} // namespace core
