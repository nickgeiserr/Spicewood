#include "pch.h"
#include "Scripting.h"
#include <Core/Console.h>

namespace Scripting {
    sol::table InitializeTable(sol::state& state) {
        sol::table spicewood = state.create_named_table("spicewood");

        spicewood.set_function("log", [](const std::string& message) {
            Print(PrintType::Info, message);
            });
        spicewood.set_function("debug", [](const std::string& message) {
            Print(PrintType::Debug, message);
            });
        spicewood.set_function("warning", [](const std::string& message) {
            Print(PrintType::Warning, message);
            });
        spicewood.set_function("error", [](const std::string& message) {
            Print(PrintType::Error, message);
            });

        return spicewood;
    }

}

namespace Mod {
    Scripting::LoadResult Mod::initialize() {
        modState.open_libraries(
            sol::lib::base,
            sol::lib::coroutine,
            sol::lib::string,
            sol::lib::utf8,
            sol::lib::table,
            sol::lib::math
        );

        sol::table spicewood = Scripting::InitializeTable(modState);

        auto result = modState.safe_script_file(modfile, sol::script_pass_on_error);

        if (!result.valid())
        {
            sol::error error = result;
            Print(PrintType::Error, error.what());
            return Scripting::LoadResult::Failed;
        }

        return Scripting::LoadResult::Success;
    }
}
