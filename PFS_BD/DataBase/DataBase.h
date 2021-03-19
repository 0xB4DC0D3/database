#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <array>
#include <filesystem>
#include <cpprest/json.h>
#include <cwctype>

#include "Logger.h"

class DataBase {
public:
	using uint = unsigned int;
	using json = web::json::value;
private:
	struct StaffOfSpecialistsTable;
	struct ApartmentOwnersTable;
	struct ContractsTable;
	struct ApartmentsTable;
public:
	DataBase();
private:
	std::pair<size_t, std::wstring> choose_database();
	void clear_console();
	void back_to_menu_from_print();
	void menu();
	void create_database() noexcept(false);
	void print_database();
	void print_staff();
	void print_apartment_owners();
	void print_contracts();
	void print_apartments();
	void print_field_staff(StaffOfSpecialistsTable staff, bool one_element = false);
	void print_field_apartment_owners(ApartmentOwnersTable apartment_owners, bool one_element = false);
	void print_field_contracts(ContractsTable contracts, bool one_element = false);
	void print_field_apartments(ApartmentsTable apartments, bool one_element = false);
	void read_by_id();
	void fiend_by_field();
	void put_field();
	void delete_field();
	void modify_field();
	template <typename T>
	void find_and_print(std::wstring table, std::wstring field, T value);
public:
	enum class DataBaseType {
		STAFF,
		APARTMENT_OWNERS,
		CONTRACTS,
		APARTMENTS
	};
private: 
	std::wfstream file;
	std::wstring db_filename;
	struct StaffOfSpecialistsTable {
		uint apartment_owner_id;
		uint contract_id;
	};
	struct ApartmentOwnersTable {
		uint apartment_owner_id;
		std::wstring name;
		std::wstring telephone;
	};
	struct ContractsTable {
		uint contract_id;
		double sales_amount;
		std::array<std::wstring, 2> date_range;
		double percentage_for_company;
		uint apartment_id;
	};
	struct ApartmentsTable {
		uint apartment_id;
		std::wstring address;
		double square_meters;
		uint count_of_rooms;
	};
	std::vector<std::filesystem::path> files;
	json data;

	Logger logger;
};