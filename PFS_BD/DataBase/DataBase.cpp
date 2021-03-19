#include "DataBase.h"

DataBase::DataBase() : logger() {
	file.imbue(std::locale(""));
	menu();
}

std::pair<size_t, std::wstring> DataBase::choose_database() {
	uint i{};
	for (auto& entry : files) {
		std::wcout << "   \x1b[33m" << ++i << ". " << entry.filename().generic_wstring() << std::endl;
	}

	if (!i) {
		throw std::wstring{ L"Нема файлів бази даних!" };
	}

	std::function<std::pair<size_t, std::wstring>()> choose = [this, &choose]() {
		std::wcout << L"\x1b[37mВиберіть базу даних: ";
		std::wstring input;
		std::wcin >> input;

		auto value{ static_cast<size_t>(std::stoi(input)) };
		if (value > files.size()) {
			choose();
		}

		return std::make_pair(value, input);
	};

	return choose();
}

void DataBase::clear_console() {
	std::wcout << L"\x1b[2J\x1b[H";
}

void DataBase::back_to_menu_from_print() {
	std::wcout << L"\x1b[0mПовернутись до попереднього меню (Y/N): ";

	std::wstring input;
	std::wcin >> input;

	clear_console();
	file.close();
	if (std::towupper(input[0]) == L'Y') {
		print_database();
	}
	else {
		menu();
	}
}

void DataBase::menu() {
	clear_console();
	std::wcout << L"\t\t\x1b[36mCompany Database\x1b[0m" << std::endl << std::endl;
	std::wcout
		<< L"   \x1b[33m1. Створити файл бази даних" << std::endl
		<< L"   \x1b[33m2. Друкувати базу даних" << std::endl
		<< L"   \x1b[33m3. Читати запис за введеним ключем" << std::endl
		<< L"   \x1b[33m4. Знайти запис за вказаним полем" << std::endl
		<< L"   \x1b[33m5. Вставити запис" << std::endl
		<< L"   \x1b[33m6. Видалити запис" << std::endl
		<< L"   \x1b[33m7. Модифікувати запис" << std::endl
		<< L"   \x1b[33m8. Вихід" << std::endl
		<< L"\x1b[37mВиберіть операцію:\x1b[0m ";

	std::wstring path{ std::filesystem::current_path().generic_wstring() + L"/databases" };
	files.clear();
	for (auto& entry : std::filesystem::directory_iterator(path)) {
		if (!std::filesystem::is_directory(entry.path())) {
			files.push_back(entry.path());
		}
	}

	std::wstring input;
	std::wcin >> input;
	switch (input[0]) {
	case L'1':
		create_database();
		break;
	case L'2':
		print_database();
		break;
	case L'3':
		read_by_id();
		break;
	case L'4':
		fiend_by_field();
		break;
	case L'5':
		put_field();
		break;
	case L'6':
		delete_field();
		break;
	case L'7':
		modify_field();
		break;
	case L'8':
		std::exit(0);
		break;
	default:
		clear_console();
		menu();
		break;
	}
}

void DataBase::create_database() noexcept(false) {
	clear_console();
	std::wcout << L"\t\t\x1b[36mСтворення файлу бази даних" << std::endl;
	std::wcout << L"\x1b[37mВведіть назву файлу БД: ";
	std::wcin >> db_filename;

	auto full_path{ std::filesystem::current_path().generic_wstring() + L"/databases/" + db_filename + L".db" };

	if (std::filesystem::exists(full_path)) 
		std::wcout << L"Такий файл вже існує!" << std::endl;
	else 
		logger.write(L"Створено файл бази даних з назвою \"" + db_filename + L"\"");

	file.open(full_path, std::ios::out);
	if (!file) 
		throw std::wstring{ L"Неможливо створити файл!" };

	json data;
	data[L"staff_of_specialists"] = json::array();
	data[L"apartment_owners"] = json::array();
	data[L"contracts"] = json::array();
	data[L"apartments"] = json::array();
	data.serialize(file);
	file.close();

	std::wcout << L"\x1b[0mПовернутись до попереднього меню (Y/N): ";

	std::wstring input;
	std::wcin >> input;

	if (std::towupper(input[0]) == L'Y') {
		create_database();
	} else {
		menu();
	}
}

void DataBase::print_database() {
	clear_console();
	std::wcout << L"\t\t\x1b[36mДрукування бази даних" << std::endl;

	auto value{ choose_database() };

	db_filename = files.at(value.first - 1);
	file.open(db_filename, std::ios::in);
	data = json::parse(file);

	std::function<void()> print_operations = [this, &input = value.second, &print_operations]() {
		clear_console();
		std::wstring filename{ std::find(db_filename.rbegin(), db_filename.rend(), L'\\').base(), db_filename.end() };
		std::wcout
			<< L"\t\t\x1b[36mБаза даних \"" << filename << L"\"" << std::endl
			<< L"   \x1b[33m1. Друкувати таблицю штабу спеціалістів" << std::endl
			<< L"   2. Друкувати таблицю власників квартир" << std::endl
			<< L"   3. Друкувати таблицю контрактів" << std::endl
			<< L"   4. Друкувати таблицю квартир" << std::endl;
		std::wcout << L"\x1b[37mВиберіть таблицю: ";
		std::wcin >> input;

		auto table{ static_cast<DataBaseType>(std::stoi(input) - 1) };
		switch (table) {
		case DataBaseType::STAFF:
			print_staff();
			break;
		case DataBaseType::APARTMENT_OWNERS:
			print_apartment_owners();
			break;
		case DataBaseType::CONTRACTS:
			print_contracts();
			break;
		case DataBaseType::APARTMENTS:
			print_apartments();
			break;
		default:
			print_operations();
			break;
		}

		back_to_menu_from_print();
	};
	
	print_operations();
	file.close();
}

void DataBase::print_staff() {
	clear_console();
	std::wcout 
		<< L"   ┌" << std::setfill(L'─') << std::setw(20) << L"─" 
		<< L"┬" << std::setfill(L'─') << std::setw(20) << L"─" << L"┐" << std::endl;
	std::wcout 
		<< L"   │" << std::setfill(L' ') << std::left << std::setw(20) << L"ApartmentOwnerID" 
		<< L"│" << std::left << std::setw(20) << L"ContractID" << L"│" << std::endl;
	
	auto make_db = [this]() {
		std::vector<StaffOfSpecialistsTable> staffs{};
		auto staff_array = data.at(L"staff_of_specialists").as_array();

		for (const auto& object : staff_array) {
			staffs.emplace_back<StaffOfSpecialistsTable>({ 
				static_cast<uint>(object.at(L"apartment_owner_id").as_integer()), 
				static_cast<uint>(object.at(L"contract_id").as_integer())
			});
		}
		
		return staffs;
	};
	for (const auto& it : make_db()) {
		print_field_staff(it);
	}

	std::wcout << L"   └" << std::setfill(L'─') << std::setw(20) << L"─" << L"┴" << std::setfill(L'─') << std::setw(20) << L"─" << L"┘" << std::endl;
	std::wcout << L"\t\t\x1b[36mКількість записів: " << data.at(L"staff_of_specialists").as_array().size() << std::endl;

	logger.write(L"Роздруковано таблицю штабу спеціалістів з бази даних \"" + db_filename + L"\"");
}

void DataBase::print_apartment_owners() {
	clear_console();
	std::wcout 
		<< L"   ┌" << std::setfill(L'─') << std::setw(20) << L"─" 
		<< L"┬" << std::setfill(L'─') << std::setw(20) << L"─" 
		<< L"┬" << std::setfill(L'─') << std::setw(20) << L"─" << L"┐" << std::endl;
	std::wcout 
		<< L"   │" << std::setfill(L' ') << std::left << std::setw(20) << L"ApartmentOwnerID" 
		<< L"│" << std::left << std::setw(20) << L"Name" 
		<< L"│" << std::left << std::setw(20) << L"Telephone" << L"│" << std::endl;

	auto make_db = [this]() {
		std::vector<ApartmentOwnersTable> staffs{};
		auto staff_array = data.at(L"apartment_owners").as_array();

		for (const auto& object : staff_array) {
			staffs.emplace_back<ApartmentOwnersTable>({
				static_cast<uint>(object.at(L"apartment_owner_id").as_integer()),
				object.at(L"name").as_string(),
				object.at(L"telephone").as_string()
			});
		}

		return staffs;
	};
	for (const auto& it : make_db()) {
		print_field_apartment_owners(it);
	}

	std::wcout 
		<< L"   └" << std::setfill(L'─') << std::setw(20) << L"─" 
		<< L"┴" << std::setfill(L'─') << std::setw(20) << L"─" 
		<< L"┴" << std::setfill(L'─') << std::setw(20) << L"─" << L"┘" << std::endl;
	std::wcout << L"\t\t\x1b[36mКількість записів: " << data.at(L"apartment_owners").as_array().size() << std::endl;

	logger.write(L"Роздруковано таблицю власників квартир з бази даних \"" + db_filename + L"\"");
}

void DataBase::print_contracts() {
	clear_console();
	std::wcout
		<< L"   ┌" << std::setfill(L'─') << std::setw(20) << L"─"
		<< L"┬" << std::setfill(L'─') << std::setw(20) << L"─"
		<< L"┬" << std::setfill(L'─') << std::setw(20) << L"─"
		<< L"┬" << std::setfill(L'─') << std::setw(20) << L"─"
		<< L"┬" << std::setfill(L'─') << std::setw(20) << L"─"
		<< L"┬" << std::setfill(L'─') << std::setw(20) << L"─" << L"┐" << std::endl;
	std::wcout
		<< L"   │" << std::setfill(L' ') << std::left << std::setw(20) << L"ContractID"
		<< L"│" << std::left << std::setw(20) << L"SalesAmount"
		<< L"│" << std::left << std::setw(20) << L"DateFrom"
		<< L"│" << std::left << std::setw(20) << L"DateTo"
		<< L"│" << std::left << std::setw(20) << L"PercentageForCompany"
		<< L"│" << std::left << std::setw(20) << L"ApartmentID" << L"│" << std::endl;

	auto make_db = [this]() {
		std::vector<ContractsTable> staffs{};
		auto staff_array = data.at(L"contracts").as_array();

		for (const auto& object : staff_array) {
			staffs.emplace_back<ContractsTable>({
				static_cast<uint>(object.at(L"contract_id").as_integer()),
				object.at(L"sales_amount").as_double(),
				std::array<std::wstring, 2> { object.at(L"date_from").as_string(), object.at(L"date_to").as_string() },
				object.at(L"percentage_for_company").as_double(),
				static_cast<uint>(object.at(L"apartment_id").as_integer())
			});
		}

		return staffs;
	};
	for (const auto& it : make_db()) {
		print_field_contracts(it);
	}

	std::wcout
		<< L"   └" << std::setfill(L'─') << std::setw(20) << L"─"
		<< L"┴" << std::setfill(L'─') << std::setw(20) << L"─"
		<< L"┴" << std::setfill(L'─') << std::setw(20) << L"─"
		<< L"┴" << std::setfill(L'─') << std::setw(20) << L"─"
		<< L"┴" << std::setfill(L'─') << std::setw(20) << L"─"
		<< L"┴" << std::setfill(L'─') << std::setw(20) << L"─" << L"┘" << std::endl;
	std::wcout << L"\t\t\x1b[36mКількість записів: " << data.at(L"contracts").as_array().size() << std::endl;

	logger.write(L"Роздруковано таблицю контрактів з бази даних \"" + db_filename + L"\"");
}

void DataBase::print_apartments() {
	clear_console();
	std::wcout
		<< L"   ┌" << std::setfill(L'─') << std::setw(20) << L"─"
		<< L"┬" << std::setfill(L'─') << std::setw(20) << L"─"
		<< L"┬" << std::setfill(L'─') << std::setw(20) << L"─"
		<< L"┬" << std::setfill(L'─') << std::setw(20) << L"─" << L"┐" << std::endl;
	std::wcout
		<< L"   │" << std::setfill(L' ') << std::left << std::setw(20) << L"ApartmentID"
		<< L"│" << std::left << std::setw(20) << L"Address"
		<< L"│" << std::left << std::setw(20) << L"SquareMeters"
		<< L"│" << std::left << std::setw(20) << L"CountOfRooms" << L"│" << std::endl;

	auto make_db = [this]() {
		std::vector<ApartmentsTable> staffs{};
		auto staff_array = data.at(L"apartments").as_array();

		for (const auto& object : staff_array) {
			staffs.emplace_back<ApartmentsTable>({
				static_cast<uint>(object.at(L"apartment_id").as_integer()),
				object.at(L"address").as_string(),
				object.at(L"square_meters").as_double(),
				static_cast<uint>(object.at(L"square_meters").as_integer())
			});
		}

		return staffs;
	};
	for (const auto& it : make_db()) {
		print_field_apartments(it);
	}

	std::wcout
		<< L"   └" << std::setfill(L'─') << std::setw(20) << L"─"
		<< L"┴" << std::setfill(L'─') << std::setw(20) << L"─"
		<< L"┴" << std::setfill(L'─') << std::setw(20) << L"─"
		<< L"┴" << std::setfill(L'─') << std::setw(20) << L"─" << L"┘" << std::endl;
	std::wcout << L"\t\t\x1b[36mКількість записів: " << data.at(L"apartments").as_array().size() << std::endl;

	logger.write(L"Роздруковано таблицю квартир з бази даних \"" + db_filename + L"\"");
}

void DataBase::print_field_staff(StaffOfSpecialistsTable staff, bool one_element) {
	if (one_element) {
		std::wcout
			<< L"   ┌" << std::setfill(L'─') << std::setw(20) << L"─"
			<< L"┬" << std::setfill(L'─') << std::setw(20) << L"─" << L"┐" << std::endl;
		std::wcout
			<< L"   │" << std::setfill(L' ') << std::left << std::setw(20) << L"ApartmentOwnerID"
			<< L"│" << std::left << std::setw(20) << L"ContractID" << L"│" << std::endl;
	}

	std::wcout
		<< L"   ├" << std::setfill(L'─') << std::setw(20) << L"─"
		<< L"┼" << std::setfill(L'─') << std::setw(20) << L"─" << L"┤" << std::endl;
	std::wcout
		<< L"   │" << std::setfill(L' ') << std::left << std::setw(20) << staff.apartment_owner_id
		<< L"│" << std::left << std::setw(20) << staff.contract_id << L"│" << std::endl;

	if (one_element) {
		std::wcout << L"   └" << std::setfill(L'─') << std::setw(20) << L"─" << L"┴" << std::setfill(L'─') << std::setw(20) << L"─" << L"┘" << std::endl;
	}
}

void DataBase::print_field_apartment_owners(ApartmentOwnersTable apartment_owners, bool one_element) {
	if (one_element) {
		std::wcout
			<< L"   ┌" << std::setfill(L'─') << std::setw(20) << L"─"
			<< L"┬" << std::setfill(L'─') << std::setw(20) << L"─"
			<< L"┬" << std::setfill(L'─') << std::setw(20) << L"─" << L"┐" << std::endl;
		std::wcout
			<< L"   │" << std::setfill(L' ') << std::left << std::setw(20) << L"ApartmentOwnerID"
			<< L"│" << std::left << std::setw(20) << L"Name"
			<< L"│" << std::left << std::setw(20) << L"Telephone" << L"│" << std::endl;
	}

	std::wcout
		<< L"   ├" << std::setfill(L'─') << std::setw(20) << L"─"
		<< L"┼" << std::setfill(L'─') << std::setw(20) << L"─"
		<< L"┼" << std::setfill(L'─') << std::setw(20) << L"─" << L"┤" << std::endl;
	std::wcout
		<< L"   │" << std::setfill(L' ') << std::left << std::setw(20) << apartment_owners.apartment_owner_id
		<< L"│" << std::left << std::setw(20) << apartment_owners.name
		<< L"│" << std::left << std::setw(20) << apartment_owners.telephone << L"│" << std::endl;

	if (one_element) {
		std::wcout
			<< L"   └" << std::setfill(L'─') << std::setw(20) << L"─"
			<< L"┴" << std::setfill(L'─') << std::setw(20) << L"─"
			<< L"┴" << std::setfill(L'─') << std::setw(20) << L"─" << L"┘" << std::endl;
	}
}

void DataBase::print_field_contracts(ContractsTable contracts, bool one_element) {
	if (one_element) {
		std::wcout
			<< L"   ┌" << std::setfill(L'─') << std::setw(20) << L"─"
			<< L"┬" << std::setfill(L'─') << std::setw(20) << L"─"
			<< L"┬" << std::setfill(L'─') << std::setw(20) << L"─"
			<< L"┬" << std::setfill(L'─') << std::setw(20) << L"─"
			<< L"┬" << std::setfill(L'─') << std::setw(20) << L"─"
			<< L"┬" << std::setfill(L'─') << std::setw(20) << L"─" << L"┐" << std::endl;
		std::wcout
			<< L"   │" << std::setfill(L' ') << std::left << std::setw(20) << L"ContractID"
			<< L"│" << std::left << std::setw(20) << L"SalesAmount"
			<< L"│" << std::left << std::setw(20) << L"DateFrom"
			<< L"│" << std::left << std::setw(20) << L"DateTo"
			<< L"│" << std::left << std::setw(20) << L"PercentageForCompany"
			<< L"│" << std::left << std::setw(20) << L"ApartmentID" << L"│" << std::endl;
	}

	std::wcout
		<< L"   ├" << std::setfill(L'─') << std::setw(20) << L"─"
		<< L"┼" << std::setfill(L'─') << std::setw(20) << L"─"
		<< L"┼" << std::setfill(L'─') << std::setw(20) << L"─"
		<< L"┼" << std::setfill(L'─') << std::setw(20) << L"─"
		<< L"┼" << std::setfill(L'─') << std::setw(20) << L"─"
		<< L"┼" << std::setfill(L'─') << std::setw(20) << L"─" << L"┤" << std::endl;
	std::wcout
		<< L"   │" << std::setfill(L' ') << std::left << std::setw(20) << contracts.contract_id
		<< L"│" << std::left << std::setw(20) << contracts.sales_amount
		<< L"│" << std::left << std::setw(20) << contracts.date_range[0]
		<< L"│" << std::left << std::setw(20) << contracts.date_range[1]
		<< L"│" << std::left << std::setw(20) << contracts.percentage_for_company
		<< L"│" << std::left << std::setw(20) << contracts.apartment_id << L"│" << std::endl;

	if (one_element) {
		std::wcout
			<< L"   └" << std::setfill(L'─') << std::setw(20) << L"─"
			<< L"┴" << std::setfill(L'─') << std::setw(20) << L"─"
			<< L"┴" << std::setfill(L'─') << std::setw(20) << L"─"
			<< L"┴" << std::setfill(L'─') << std::setw(20) << L"─"
			<< L"┴" << std::setfill(L'─') << std::setw(20) << L"─"
			<< L"┴" << std::setfill(L'─') << std::setw(20) << L"─" << L"┘" << std::endl;
	}
}

void DataBase::print_field_apartments(ApartmentsTable apartments, bool one_element) {
	if (one_element) {
		std::wcout
			<< L"   ┌" << std::setfill(L'─') << std::setw(20) << L"─"
			<< L"┬" << std::setfill(L'─') << std::setw(20) << L"─"
			<< L"┬" << std::setfill(L'─') << std::setw(20) << L"─"
			<< L"┬" << std::setfill(L'─') << std::setw(20) << L"─" << L"┐" << std::endl;
		std::wcout
			<< L"   │" << std::setfill(L' ') << std::left << std::setw(20) << L"ApartmentID"
			<< L"│" << std::left << std::setw(20) << L"Address"
			<< L"│" << std::left << std::setw(20) << L"SquareMeters"
			<< L"│" << std::left << std::setw(20) << L"CountOfRooms" << L"│" << std::endl;
	}

	std::wcout
		<< L"   ├" << std::setfill(L'─') << std::setw(20) << L"─"
		<< L"┼" << std::setfill(L'─') << std::setw(20) << L"─"
		<< L"┼" << std::setfill(L'─') << std::setw(20) << L"─"
		<< L"┼" << std::setfill(L'─') << std::setw(20) << L"─" << L"┤" << std::endl;
	std::wcout
		<< L"   │" << std::setfill(L' ') << std::left << std::setw(20) << apartments.apartment_id
		<< L"│" << std::left << std::setw(20) << apartments.address
		<< L"│" << std::left << std::setw(20) << apartments.square_meters
		<< L"│" << std::left << std::setw(20) << apartments.count_of_rooms << L"│" << std::endl;

	if (one_element) {
		std::wcout
			<< L"   └" << std::setfill(L'─') << std::setw(20) << L"─"
			<< L"┴" << std::setfill(L'─') << std::setw(20) << L"─"
			<< L"┴" << std::setfill(L'─') << std::setw(20) << L"─"
			<< L"┴" << std::setfill(L'─') << std::setw(20) << L"─" << L"┘" << std::endl;
	}
}

void DataBase::read_by_id() {
	clear_console();
	std::wcout << L"\t\t\x1b[36mЧитання запису за ключем" << std::endl;

	auto value{ choose_database() };

	db_filename = files.at(value.first - 1);
	file.open(db_filename, std::ios::in);
	json data{ json::parse(file) };

	std::wcout << L"\x1b[0mВведіть ID контракту: ";
	std::wcin >> value.second;

	auto id_for_read{ std::stoi(value.second) };

	for (size_t i = 0; i < data.at(L"staff_of_specialists").as_array().size(); ++i) {
		auto index{ data.at(L"staff_of_specialists")[i].at(L"contract_id").as_integer() };
		if (index == id_for_read) {
			clear_console();
			std::wstring filename{ std::find(db_filename.rbegin(), db_filename.rend(), L'\\').base(), db_filename.end() };
			std::wcout << L"\t\t\x1b[36mПошук за ім'ям власника квартири в \"" << filename << L"\"\x1b[0m" << std::endl;

			print_field_staff(StaffOfSpecialistsTable {
				static_cast<uint>(data.at(L"staff_of_specialists")[i].at(L"apartment_owner_id").as_integer()),
				static_cast<uint>(data.at(L"staff_of_specialists")[i].at(L"contract_id").as_integer())
			}, true);
			print_field_apartment_owners(ApartmentOwnersTable{
				static_cast<uint>(data.at(L"apartment_owners")[i].at(L"apartment_owner_id").as_integer()),
				data.at(L"apartment_owners")[i].at(L"name").as_string(),
				data.at(L"apartment_owners")[i].at(L"telephone").as_string()
			}, true);
			print_field_contracts(ContractsTable{
				static_cast<uint>(data.at(L"contracts")[i].at(L"contract_id").as_integer()),
				data.at(L"contracts")[i].at(L"sales_amount").as_double(),
				std::array<std::wstring, 2> { data.at(L"contracts")[i].at(L"date_from").as_string(), data.at(L"contracts")[i].at(L"date_to").as_string() },
				data.at(L"contracts")[i].at(L"percentage_for_company").as_double(),
				static_cast<uint>(data.at(L"contracts")[i].at(L"apartment_id").as_integer())
			}, true);
			print_field_apartments(ApartmentsTable{
				static_cast<uint>(data.at(L"apartments")[i].at(L"apartment_id").as_integer()),
				data.at(L"apartments")[i].at(L"address").as_string(),
				data.at(L"apartments")[i].at(L"square_meters").as_double(),
				static_cast<uint>(data.at(L"apartments")[i].at(L"square_meters").as_integer())
			}, true);
			
			std::wcout << L"\x1b[32mЗапис з ідентифікатором " << index << L" знайдено!\x1b[0m" << std::endl;
			logger.write((std::wstringstream{} << L"Читання запису за ключем \"" << id_for_read << L"\" з бази даних \"" << db_filename << L"\"").str());
			break;
		}
	}

	file.close();

	std::wcout << L"\x1b[0mПовернутись до попереднього меню (Y/N): ";
	std::wcin >> value.second;

	if (std::towupper(value.second[0]) == L'Y') {
		read_by_id();
	}
	else {
		menu();
	}
}

void DataBase::fiend_by_field() {
	clear_console();
	std::wcout << L"\t\t\x1b[36mЗнаходження запису за вказаним полем" << std::endl;

	auto value{ choose_database() };

	db_filename = files.at(value.first - 1);
	file.open(db_filename, std::ios::in);
	data = json::parse(file);

	std::function<void()> print_operations = [this, &input = value.second, &print_operations]() {
		clear_console();
		std::wstring filename{ std::find(db_filename.rbegin(), db_filename.rend(), L'\\').base(), db_filename.end() };
		std::wcout
			<< L"\t\t\x1b[36mБаза даних \"" << filename << "\"" << std::endl
			<< L"   \x1b[33m1. Знайти за ім'ям власника квартири" << std::endl
			<< L"   2. Знайти за ідентифікатором" << std::endl
			<< L"   3. Знайти за ціною квартири" << std::endl
			<< L"   4. Знайти за номером телефону" << std::endl
			<< L"   5. Знайти за квадратними метрами" << std::endl
			<< L"   5. Знайти за кількістю кімнат" << std::endl;
		std::wcout << L"\x1b[37mВиберіть пошук: ";
		std::wcin >> input;
		auto table{ std::stoi(input) };
		std::wcout << L"\x1b[37mВведіть значення для пошуку: ";
		std::getline(std::wcin >> std::ws, input);

		clear_console();
		
		switch (table) {
		case 1:
			std::wcout << L"\t\t\x1b[36mПошук за ім'ям власника квартири в \"" << filename << L"\"\x1b[0m" << std::endl;
			logger.write((std::wstringstream{} << L"Пошук запису за ім'ям власника квартири \"" << input << L"\" в базі даних \"" << db_filename << L"\"").str());
			find_and_print(L"apartment_owners", L"name", input);
			break;
		case 2:
			std::wcout << L"\t\t\x1b[36mПошук за ідентифікатором в \"" << filename << L"\"\x1b[0m" << std::endl;
			logger.write((std::wstringstream{} << L"Пошук запису за ідентифікатором \"" << input << L"\" в базі даних \"" << db_filename << L"\"").str());
			find_and_print(L"staff_of_specialists", L"contract_id", std::stoi(input));
			break;
		case 3:
			std::wcout << L"\t\t\x1b[36mПошук за ціною квартири в \"" << filename << L"\"\x1b[0m" << std::endl;
			logger.write((std::wstringstream{} << L"Пошук запису за ціною квартири \"" << input << L"\" в базі даних \"" << db_filename << L"\"").str());
			find_and_print(L"contracts", L"sales_amount", std::stod(input));
			break;
		case 4:
			std::wcout << L"\t\t\x1b[36mПошук за номером телефону в \"" << filename << L"\"\x1b[0m" << std::endl;
			logger.write((std::wstringstream{} << L"Пошук запису за номером телефону \"" << input << L"\" в базі даних \"" << db_filename << L"\"").str());
			find_and_print(L"apartment_owners", L"telephone", input);
			break;
		case 5:
			std::wcout << L"\t\t\x1b[36mПошук за квадратними метрами в \"" << filename << L"\"\x1b[0m" << std::endl;
			logger.write((std::wstringstream{} << L"Пошук запису за квадратними метрами \"" << input << L"\" в базі даних \"" << db_filename << L"\"").str());
			find_and_print(L"apartments", L"square_meters", std::stod(input));
			break;
		case 6:
			std::wcout << L"\t\t\x1b[36mПошук за кількістю кімнат в \"" << filename << L"\"\x1b[0m" << std::endl;
			logger.write((std::wstringstream{} << L"Пошук запису за кількістю кімнат \"" << input << L"\" в базі даних \"" << db_filename << L"\"").str());
			find_and_print(L"apartments", L"count_of_rooms", std::stoi(input));
			break;
		default:
			print_operations();
			break;
		}

		std::wcout << L"\x1b[0mПовернутись до попереднього меню (Y/N): ";
		std::wcin >> input;

		file.close();

		if (std::towupper(input[0]) == L'Y') {
			fiend_by_field();
		} else {
			menu();
		}
	};

	print_operations();
	file.close();
}

void DataBase::put_field() {
	clear_console();
	std::wcout << L"\t\t\x1b[36mВставлення запису у базу даних" << std::endl;

	auto value{ choose_database() };

	db_filename = files.at(value.first - 1);
	file.open(db_filename, std::ios::in);
	data = json::parse(file);
	file.close();
	file.open(db_filename, std::ios::in | std::ios::out | std::ios::trunc);

	uint id{};
	auto make_field = [this, &id]() {
		auto staff{ json::object() };
		auto apartment_owners{ json::object() };
		auto contracts{ json::object() };
		auto apartments{ json::object() };

		clear_console();
		std::wstring filename{ std::find(db_filename.rbegin(), db_filename.rend(), L'\\').base(), db_filename.end() };
		std::wcout << L"\t\t\x1b[36mБаза даних \"" << filename << "\"" << std::endl;

		std::wcout << L"\x1b[0mВведіть ім'я власника: ";
		std::wstring name;
		std::getline(std::wcin >> std::ws, name);
		apartment_owners[L"name"] = json::value(name);

		std::wcout << L"Введіть номер телефона власника: ";
		std::wstring telephone;
		std::wcin >> telephone;
		apartment_owners[L"telephone"] = json::value(telephone);

		std::wcout << L"Введіть ціну за квартиру: ";
		double sales_amount;
		std::wcin >> sales_amount;
		contracts[L"sales_amount"] = json::value(sales_amount);

		std::wcout << L"Введіть термін продажу (dd.mm.yyyy dd.mm.yyyy): ";
		std::wstring date_from, date_to;
		std::wcin >> date_from >> date_to;
		contracts[L"date_from"] = json::value(date_from);
		contracts[L"date_to"] = json::value(date_to);

		std::wcout << L"Введіть процент від продажу для компанії: ";
		double percentage_for_company;
		std::wcin >> percentage_for_company;
		contracts[L"percentage_for_company"] = json::value(percentage_for_company);

		std::wcout << L"Введіть адресу квартири: ";
		std::wstring address;
		std::getline(std::wcin >> std::ws, address);
		apartments[L"address"] = json::value(address);

		std::wcout << L"Введіть кількість кв. метрів: ";
		double square_meters;
		std::wcin >> square_meters;
		apartments[L"square_meters"] = json::value(square_meters);

		std::wcout << L"Введіть кількість кімнат: ";
		uint count_of_rooms;
		std::wcin >> count_of_rooms;
		apartments[L"count_of_rooms"] = json::value(count_of_rooms);
		logger.write((std::wstringstream{} << L"Вставленню запису з наступними значеннями \"" 
			<< L"ім'я: " << name << ", "
			<< L"телефон: " << telephone << ", "
			<< L"вартість: " << sales_amount << ", "
			<< L"з: " << date_from << ", "
			<< L"по: " << date_to << ", "
			<< L"відсоток: " << percentage_for_company << ", "
			<< L"адреса: " << address << ", "
			<< L"м.кв.: " << square_meters << ", "
			<< L"к-сть кімнат: " << count_of_rooms
			<< L"\" в базу даних \"" << db_filename << L"\"").str());

		id = data.at(L"apartment_owners").size() == 0 ? 1 : (data.at(L"apartment_owners")[data.at(L"apartment_owners").size() - 1].at(L"apartment_owner_id").as_integer() + 1);
		apartment_owners[L"apartment_owner_id"] = json::value(id);
		apartments[L"apartment_id"] = json::value(id);
		contracts[L"apartment_id"] = json::value(id);
		contracts[L"contract_id"] = json::value(id);
		staff[L"apartment_owner_id"] = json::value(id);
		staff[L"contract_id"] = json::value(id);

		data[L"staff_of_specialists"][data.at(L"staff_of_specialists").size()] = staff;
		data[L"apartment_owners"][data.at(L"apartment_owners").size()] = apartment_owners;
		data[L"contracts"][data.at(L"contracts").size()] = contracts;
		data[L"apartments"][data.at(L"apartments").size()] = apartments;
		data.serialize(file);
	};

	make_field();
	file.close();
	
	std::wcout << L"\x1b[32mЗапис з ідентифікатором " << id << L" успішно створено!\x1b[0m" << std::endl;
	std::wcout << L"\x1b[0mПовернутись до попереднього меню (Y/N): ";
	std::wcin >> value.second;

	if (std::towupper(value.second[0]) == L'Y') {
		put_field();
	} else {
		menu();
	}
}

void DataBase::delete_field() {
	clear_console();
	std::wcout << L"\t\t\x1b[36mВидалення запису з бази даних" << std::endl;
	auto value{ choose_database() };

	db_filename = files.at(value.first - 1);
	file.open(db_filename, std::ios::in);
	data = json::parse(file);
	file.close();
	file.open(db_filename, std::ios::in | std::ios::out | std::ios::trunc);

	print_contracts();
	std::wcout << L"\x1b[0mВведіть ID контракту: ";
	std::wcin >> value.second;

	auto id_for_delete{ std::stoi(value.second) };

	for (size_t i = 0; i < data.at(L"staff_of_specialists").as_array().size(); ++i) {
		auto index{ data.at(L"staff_of_specialists")[i].at(L"contract_id").as_integer() };
		if (index == id_for_delete) {
			data[L"staff_of_specialists"].erase(i);
			data[L"apartment_owners"].erase(i);
			data[L"contracts"].erase(i);
			data[L"apartments"].erase(i);
			std::wcout << L"\x1b[32mЗапис з ідентифікатором " << index << L" успішно видалено!\x1b[0m" << std::endl;
			logger.write((std::wstringstream{} << L"Видалення запису за ідентифікатором \"" << index << L"\" з бази даних \"" << db_filename << L"\"").str());
		}
	}

	data.serialize(file);
	file.close();

	std::wcout << L"\x1b[0mПовернутись до попереднього меню (Y/N): ";
	std::wcin >> value.second;

	if (std::towupper(value.second[0]) == L'Y') {
		delete_field();
	} else {
		menu();
	}
}

void DataBase::modify_field() {
	clear_console();
	std::wcout << L"\t\t\x1b[36mМодифікація бази даних" << std::endl;
	auto value{ choose_database() };

	db_filename = files.at(value.first - 1);
	file.open(db_filename, std::ios::in);
	data = json::parse(file);
	file.close();
	file.open(db_filename, std::ios::in | std::ios::out | std::ios::trunc);

	print_contracts();
	std::wcout << L"\x1b[0mВведіть ID для модифікації: ";
	std::wcin >> value.second;

	auto id_for_modify{ std::stoi(value.second) };
	std::wstring name;
	std::wstring telephone;
	double sales_amount;
	std::wstring date_from, date_to;
	double percentage_for_company;
	std::wstring address;
	double square_meters;
	uint count_of_rooms;

	std::wstring filename{ std::find(db_filename.rbegin(), db_filename.rend(), L'\\').base(), db_filename.end() };
	std::wcout << L"\t\t\x1b[36mМодифікація бази даних \"" << filename << L"\"" << std::endl;

	for (size_t i = 0; i < data.at(L"staff_of_specialists").as_array().size(); ++i) {
		auto index{ data.at(L"staff_of_specialists")[i].at(L"contract_id").as_integer() };
		if (index == id_for_modify) {
			std::wcout << L"\x1b[0mВведіть ім'я власника: ";
			std::getline(std::wcin >> std::ws, name);
			data[L"apartment_owners"][i][L"name"] = json::value(name);

			std::wcout << L"Введіть номер телефона власника: ";
			std::wcin >> telephone;
			data[L"apartment_owners"][i][L"telephone"] = json::value(telephone);

			std::wcout << L"Введіть ціну за квартиру: ";
			std::wcin >> sales_amount;
			data[L"contracts"][i][L"sales_amount"] = json::value(sales_amount);

			std::wcout << L"Введіть термін продажу (dd.mm.yyyy dd.mm.yyyy): ";
			std::wcin >> date_from >> date_to;
			data[L"contracts"][i][L"date_from"] = json::value(date_from);
			data[L"contracts"][i][L"date_to"] = json::value(date_to);

			std::wcout << L"Введіть процент від продажу для компанії: ";
			std::wcin >> percentage_for_company;
			data[L"contracts"][i][L"percentage_for_company"] = json::value(percentage_for_company);

			std::wcout << L"Введіть адресу квартири: ";
			std::getline(std::wcin >> std::ws, address);
			data[L"apartments"][i][L"address"] = json::value(address);

			std::wcout << L"Введіть кількість кв. метрів: ";
			std::wcin >> square_meters;
			data[L"apartments"][i][L"square_meters"] = json::value(square_meters);

			std::wcout << L"Введіть кількість кімнат: ";
			std::wcin >> count_of_rooms;
			data[L"apartments"][i][L"address"] = json::value(address);

			std::wcout << L"\x1b[32mЗапис з ідентифікатором " << index << L" успішно модифіковано!\x1b[0m" << std::endl;
			logger.write((std::wstringstream{} << L"Модифікація запису за ідентифікатором \"" << index << L"\" в базі даних \"" << db_filename << L"\"").str());
		}
	}

	data.serialize(file);
	file.close();

	std::wcout << L"\x1b[0mПовернутись до попереднього меню (Y/N): ";
	std::wcin >> value.second;

	if (std::towupper(value.second[0]) == L'Y') {
		modify_field();
	} else {
		menu();
	}
}

template <typename T>
void DataBase::find_and_print(std::wstring table, std::wstring field, T value) {
	auto print_all_db = [this, &value](auto i) {
		print_field_staff(StaffOfSpecialistsTable{
			static_cast<uint>(data.at(L"staff_of_specialists")[i].at(L"apartment_owner_id").as_integer()),
			static_cast<uint>(data.at(L"staff_of_specialists")[i].at(L"contract_id").as_integer())
		}, true);
		print_field_apartment_owners(ApartmentOwnersTable{
			static_cast<uint>(data.at(L"apartment_owners")[i].at(L"apartment_owner_id").as_integer()),
			data.at(L"apartment_owners")[i].at(L"name").as_string(),
			data.at(L"apartment_owners")[i].at(L"telephone").as_string()
		}, true);
		print_field_contracts(ContractsTable{
			static_cast<uint>(data.at(L"contracts")[i].at(L"contract_id").as_integer()),
			data.at(L"contracts")[i].at(L"sales_amount").as_double(),
			std::array<std::wstring, 2> { data.at(L"contracts")[i].at(L"date_from").as_string(), data.at(L"contracts")[i].at(L"date_to").as_string() },
			data.at(L"contracts")[i].at(L"percentage_for_company").as_double(),
			static_cast<uint>(data.at(L"contracts")[i].at(L"apartment_id").as_integer())
		}, true);
		print_field_apartments(ApartmentsTable{
			static_cast<uint>(data.at(L"apartments")[i].at(L"apartment_id").as_integer()),
			data.at(L"apartments")[i].at(L"address").as_string(),
			data.at(L"apartments")[i].at(L"square_meters").as_double(),
			static_cast<uint>(data.at(L"apartments")[i].at(L"count_of_rooms").as_integer())
		}, true);

		std::wcout << L"\x1b[32mЗапис зі значенням " << value << L" знайдено!\x1b[0m" << std::endl;
	};

	for (size_t i = 0; i < data.at(table).as_array().size(); ++i) {
		if constexpr (std::is_same_v<int, T>) {
			if (value == data.at(table)[i].at(field).as_integer()) {
				print_all_db(i);
				break;
			}
		} else if constexpr (std::is_same_v<double, T>) {
			auto readed_value{ data.at(table)[i].at(field).as_double() };
			if (value >= (readed_value - 0.5) || value <= (readed_value + 0.5)) {
				print_all_db(i);
				break;
			}
		} else if constexpr (std::is_same_v<std::wstring, T>) {
			if (std::wstring{ value } == data.at(table)[i].at(field).as_string()) {
				print_all_db(i);
				break;
			}
		}
	}
}