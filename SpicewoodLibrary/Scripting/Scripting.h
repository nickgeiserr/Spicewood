#pragma once
#include <sol/sol.hpp>
#include <string>

namespace Scripting {
	enum LoadResult {
		Success,
		Failed,
		MissingPrerequisite,
		NotFound
	};


	sol::table InitializeTable(sol::state& state);
}

namespace Mod {
	class Mod {
	public:
		std::string id;
		std::string name;
		std::string author;
		std::string modfile;
		std::string apiVersion;
		sol::state modState;

		inline Mod(std::string modId, std::string modName, std::string modAuthor, std::string modFileName) {
			id = modId;
			name = modName;
			author = modAuthor;
			modfile = modFileName;
		}

		Scripting::LoadResult initialize();
	};
}
