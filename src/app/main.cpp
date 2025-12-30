#include "controller/DataController.h"
#include "Log.h"
#include "UiController.h"

int main(int argc, char** argv)
{
    (void)argc; (void)argv;

    core::Log::info("FlyFF Framework - Start");

    core::DataController data;
    if (!data.initialize("."))                      // überprüft config / erstellt Ordner
        return 1;

    ui::UiController ui;
    ui.injectDataController(&data);                 // <<< DIES FEHLTE – Dialog aktiv!
    // UI weiß jetzt: ein DataController existiert

    if (!ui.initialize("FlyFF Framework", 1280, 720))
        return 1;

    ui.run();                                       // rendering loop

    core::Log::info("FlyFF Framework - Exit");
    return 0;
}
