#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include <yhirose/httplib.h>

using json = nlohmann::json;

const std::string SERVER_HOST = "0.0.0.0";
const int SERVER_PORT = 8000;
const std::string LEADERBOARD_FILE = "leaderboard.json";

json leaderboard;

/*
	name: string
	score: int
*/
void post_score(const httplib::Request& request, httplib::Response& response)
{
	std::string name = request.get_param_value("name");
	int score = std::stoi(request.get_param_value("score"));

	leaderboard.push_back({ {"name", name}, {"score", score} });
	std::sort(leaderboard.begin(), leaderboard.end(), [](const nlohmann::json& a, const nlohmann::json& b) {
		return a["score"] > b["score"];
	});

	std::ofstream file_leaderboard("leaderboard.json");
	file_leaderboard << leaderboard << std::endl;
	std::cout << std::setw(4) << leaderboard << std::endl;

	response.set_content("Score recorded", "text/plain");
}

/*
	positions_count: int
*/
void get_leadeboard(const httplib::Request& request, httplib::Response& response)
{
	// По умолчанию возвращает до 20 позиций
	int positions_count = 20;
	if (request.has_param("positions_count")) {
		positions_count = std::stoi(request.get_param_value("positions_count"));
	}

	// Выбираем количество выводимых позиций лидерборда
	// максимально 100 позиций
	positions_count = std::min(std::min(positions_count, (int)leaderboard.size()), 100);

	json leaderboard_slice = json::array();

	for (int score_position = 0; score_position < positions_count; ++score_position) {
		leaderboard_slice.push_back(leaderboard.at(score_position));
	}

	response.body = leaderboard_slice.dump(4);
}

int main()
{
	// Инициализация лидербордов
	std::fstream file_leaderboard{ LEADERBOARD_FILE };
	
	// если файл открыт, пишем его содержимое в структуру
	if (file_leaderboard.is_open()) {
		file_leaderboard >> leaderboard;
	}

	// Инициализация сервера
	httplib::Server server;
	server.Post("/score", post_score);
	server.Get("/leaderboard", get_leadeboard);

	server.set_exception_handler([&](const httplib::Request&, httplib::Response& response, std::exception_ptr ex) {
		response.body = "Exception occured";
	});

	// Запуск сервера
	std::cout << "Server stared on " << SERVER_HOST << ':' << SERVER_PORT << std::endl;
	try {
		server.listen(SERVER_HOST, SERVER_PORT);
	}
	catch (std::exception e)
	{
		std::cerr << e.what() << std::endl;
	}
}