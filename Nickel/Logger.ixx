export module Logger;

import <string>;
import <source_location>;
import <sstream>;

export namespace Logger {
	auto GetSourceLocation(const std::source_location& loc) -> std::string {
		std::ostringstream result("file: ", std::ios_base::ate);
    	result << loc.file_name() << std::endl
			   << "function: '" << loc.function_name() << "'" << std::endl
			   << "line: " << loc.line();
			   
		return result.str();
	}
}