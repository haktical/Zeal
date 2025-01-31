#include "commands.h"
#include "EqStructures.h"
#include "EqAddresses.h"
#include "EqFunctions.h"
#include "Zeal.h"
#include <algorithm>
#include <cctype>
#include "StringUtil.h"




void __fastcall InterpretCommand(int c, int unused, int player, char* cmd)
{
	ZealService* zeal = ZealService::get_instance();
	std::string str_cmd = cmd;
	std::vector<std::string> args =  StringUtil::split(str_cmd," ");

	if (args.size() > 0)
	{
		bool cmd_handled = false;
		if (zeal->commands_hook->CommandFunctions.count(args[0]) > 0)
		{
			cmd_handled = zeal->commands_hook->CommandFunctions[args[0]].callback(args);
		}
		else
		{
			for (auto& m : zeal->commands_hook->CommandFunctions)
			{
				for (auto& a : m.second.aliases)
				{
					if (a == args[0])
					{
						cmd_handled = m.second.callback(args);
						break;
					}
				}
			}
		}
		if (cmd_handled) {
			cmd[0] = '\0';
			return;
			return;
		}
	}
	zeal->hooks->hook_map["commands"]->original(InterpretCommand)(c, unused, player, cmd);
}

void ChatCommands::add(std::string cmd, std::vector<std::string>aliases, std::function<bool(std::vector<std::string>&args)> callback)
{
	CommandFunctions[cmd] = ZealCommand(aliases, callback);
}

ChatCommands::~ChatCommands()
{

}
ChatCommands::ChatCommands(ZealService* zeal)
{

	//just going to use lambdas for simple commands
	add("/autoinventory", { "/autoinv", "/ai" },
		[](std::vector<std::string>& args) {
			int a1 = *Zeal::EqGame::ptr_LocalPC;
			int a2 = a1 + 0xD78;
			Zeal::EqGame::EqGameInternal::auto_inventory(a1, a2, 0);
			return true; //return true to stop the game from processing any further on this command, false if you want to just add features to an existing cmd
		});
	add("/camp", {},
		[](std::vector<std::string>& args) {

			Zeal::EqGame::change_stance(Stance::Sit);
			return false;
		});
	add("/showlootlockouts", {"/sll", "/showlootlockout", "/showlockouts"},
		[](std::vector<std::string>& args) {

			Zeal::EqGame::do_say(true, "#showlootlockouts");
			return true;
		});
	add("/zeal", { "/zea" },
		[this](std::vector<std::string>& args) {
			if (args.size() == 1)
			{
				std::stringstream ss;
				Zeal::EqGame::print_chat("Available args: version"); //leave room for more args on this command for later
				return true;
			}
			if (args.size() > 1 && StringUtil::caseInsensitive(args[1], "version"))
			{
				std::stringstream ss;
				ss << "Zeal version: " << ZEAL_VERSION << std::endl;
				Zeal::EqGame::print_chat(ss.str());
				return true;
			}
			return false;

		});
	add("/help", {"/hel"},
		[this](std::vector<std::string>& args) {
			if (args.size() == 1)
			{
				std::stringstream ss;
				ss << "Format: /help <class> Where class is one of normal, emote, guild, ect.." << std::endl;
				ss << "Normal will display a list of all commands." << std::endl;
				ss << "Emote will display a list of all player emotes." << std::endl;
				ss << "Guild will display a list of guild commands." << std::endl;
				ss << "Voice will display a list of voice control commands." << std::endl;
				ss << "Chat will display a list of chat channel commands." << std::endl;
				ss << "Zeal will display a list of custom commands." << std::endl;
				Zeal::EqGame::print_chat(ss.str());
				return true;
			}
			if (args.size() > 1 && StringUtil::caseInsensitive(args[1],"zeal"))
			{
				std::stringstream ss;
				ss << "List of commands" << std::endl;
				ss << "-----------------------------------------------------" << std::endl;
				for (auto& [name, c] : CommandFunctions)
				{
					ss << name;
					if (c.aliases.size() > 0)
						ss << "  aliases  [";
					for (auto it = c.aliases.begin(); it != c.aliases.end(); ++it) {
						auto& a = *it;
						ss << a;
						if (std::next(it) != c.aliases.end()) {
							ss << ", ";
						}
					}
					if (c.aliases.size() > 0)
						ss << "]";
					ss << std::endl;
				}
				Zeal::EqGame::print_chat(ss.str());
				return true;
			}
			return false;
			
		});
	zeal->hooks->Add("commands", Zeal::EqGame::EqGameInternal::fn_interpretcmd, InterpretCommand, hook_type_detour);
}

