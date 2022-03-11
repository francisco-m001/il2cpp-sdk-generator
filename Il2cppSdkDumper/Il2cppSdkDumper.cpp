#include <iostream>
#include <windows.h>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <nlohmann/json.hpp>

#define PRINT_OFFSET(x) printf("%s = 0x%X;\n", #x, x);
#define IL2CPP_DUMPER_FOLDER std::string("E:\\il2cppdumper\\")
#define IL2CPP_SCRIPT_PATH IL2CPP_DUMPER_FOLDER + "\\script.json" 
#define RUST_FOLDER std::string("E:\\SteamLibrary\\steamapps\\common\\Rust\\")

bool GetConsoleVariable( )
{
	std::string out;
	std::cin >> out;
	if ( out.find( "y" ) != std::string::npos )
		return true;
	return false;
}

int fileSize( std::string filename )
{
	std::ifstream in( filename, std::ifstream::ate | std::ifstream::binary );
	return in.tellg( );
}

void __f__AnonymousType0_object__object__bool____ctor( void* __this, void* Identifier, void* Name, bool State, const void* method )
{
	static void ( *__f__AnonymousType0_object__object__bool____ctor ) ( void* __this, void* Identifier, void* Name, bool State, const void* method ) = reinterpret_cast< decltype( __f__AnonymousType0_object__object__bool____ctor ) > ( 22210208 );
	
	//return __f__AnonymousType0_object__object__bool____ctor( void* __this, void* Identifier, void* Name, bool State, const void* method );
}

int main( )
{
	//Run il2cppdumper to get latest metadata
	printf( "Run dumper (y/n)\n" );
	if ( GetConsoleVariable( ) )
	{
		printf( "Running il2cpp dumper...\n" );
		system( std::string( "" + IL2CPP_DUMPER_FOLDER + "Il2CppDumper.exe " + RUST_FOLDER + "GameAssembly.dll " + RUST_FOLDER + "RustClient_Data\\il2cpp_data\\Metadata\\global-metadata.dat " + IL2CPP_DUMPER_FOLDER ).c_str( ) );
	}

	std::ifstream script_json( IL2CPP_SCRIPT_PATH );
	const auto FileSize = fileSize( IL2CPP_SCRIPT_PATH );
	if ( !FileSize )
	{
		printf( ( "Invalid script json file...\n" ) );
		system( "pause" );
		return 0;
	}

	std::string fileContent;
	if ( script_json.is_open( ) )
	{
		std::string line;
		while ( getline( script_json, line ) )
		{
			fileContent += line;
		}

		script_json.close( );
	}

	if ( !nlohmann::json::accept( fileContent ) )
	{
		printf( ( "File is invalid...\n" ) );
		system( "pause" );
		return 0;
	}

	struct ScriptMethod
	{
		uint64_t Address = 0;
		std::string Signature;
	};

	std::map<std::string, ScriptMethod> ScriptMethodMap = { };

	auto json_file = nlohmann::json::parse( fileContent );
	auto ScriptMethod = json_file[ "ScriptMethod" ];
	if ( !ScriptMethod.is_null( ) )
	{
		for ( auto scriptMethod : ScriptMethod )
		{
			if ( scriptMethod.is_null( ) || scriptMethod.is_object( ) )
				ScriptMethodMap[ scriptMethod[ "Name" ] ] = { scriptMethod[ "Address" ], scriptMethod[ "Signature" ] };
		}
	}

	std::ofstream MethodHeaderFile( "Methods.h" );
	if ( MethodHeaderFile.is_open( ) )
	{
		MethodHeaderFile << "#pragma once\n\n";
		MethodHeaderFile << "#include \"il2cpp.h\"\n\n";
		for ( const auto [MethodName, MethodStruct] : ScriptMethodMap )
		{
			MethodHeaderFile << MethodStruct.Signature << std::endl;
		}

		MethodHeaderFile.close( );
	}

	std::ofstream MethodCodeFile( "Methods.cpp" );
	if ( MethodCodeFile.is_open( ) )
	{
		MethodCodeFile << "#include \"Methods.h\"\n\n";
		for ( const auto [MethodName, MethodStruct] : ScriptMethodMap )
		{
			auto localMethodSignature = MethodStruct.Signature;
			localMethodSignature.pop_back( );
			MethodCodeFile << localMethodSignature << std::endl;

			auto methodTypeDef = localMethodSignature;
			const auto firstBracketLocation = methodTypeDef.find_first_of( "(" );
			const auto firstSpaceLocation = methodTypeDef.find_first_of( " " );
			const auto cleanMethodName = std::string( methodTypeDef.begin( ) + firstSpaceLocation, methodTypeDef.begin( ) + firstBracketLocation );
			auto functionArgs = std::string( methodTypeDef.begin( ) + firstBracketLocation, methodTypeDef.end( ) );

			methodTypeDef.insert( firstSpaceLocation + 1, "(*" );
			methodTypeDef.insert( firstBracketLocation + 1, ")" );
			methodTypeDef.insert( 0, "static " );
			methodTypeDef += " = reinterpret_cast< decltype( " + cleanMethodName + " )> ( GetModuleBaseAddress(\"GameAssembly.dll\") + " + std::to_string( MethodStruct.Address ) +" ); ";

			std::vector<std::string> functionArgsFiltered;
			for ( int i = 0; i < functionArgs.size( ); i++ )
			{
				if ( functionArgs[ i ] == ',' )
				{
					std::string argName = std::string( functionArgs.begin( ) + functionArgs.find_last_of( " ", i ), functionArgs.begin( ) + i );
					const auto spacePosition = argName.find( " " );
					if ( spacePosition != std::string::npos )
						argName.erase( spacePosition, spacePosition + 1 );

					functionArgsFiltered.push_back( argName );
				}
			}
			functionArgsFiltered.push_back( "method" );
			auto returnStatement = "return " + cleanMethodName + "(";
			for ( int i = 0; i < functionArgsFiltered.size( ); i++ )
			{
				std::string comma = i != functionArgsFiltered.size( ) - 1 ? ", " : "";
				returnStatement += functionArgsFiltered[ i ] + comma;
			}
			returnStatement += ");";

			MethodCodeFile << "{\n";
			MethodCodeFile << "\t" + methodTypeDef << std::endl;
			MethodCodeFile << "\t" + returnStatement << std::endl;
			MethodCodeFile << "}\n";
		}

		MethodCodeFile.close( );
	}
	std::cout << "Hello World!\n";
}