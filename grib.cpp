#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
using namespace std;


int main()
{
	fstream grib;
	//Otwarcie pliku w trybie do odczytu oraz trybie binarnym, koniecznie użyć binarnego or - | do złączenia tych flag
	grib.open("D:\\StudiaCC\\StudiaCC\\all.grib", std::ios::in | std::ios::binary);

	if (!grib.is_open())
	{
		cout << "Nie udalo sie otworzyc pliku!\n";
		return -1;
	}

	ofstream outputFile("output.txt");

	if (!outputFile.is_open())
	{
		cout << "Nie udalo sie otworzyc pliku do zapisu!\n";
		return -1;
	}

	stringstream outputBuffer;

	uint32_t start_section_0 = 0;

	uint8_t byte;
	uint8_t byte2;
	uint8_t byte3;
	uint8_t byte4;
	char byte_char;


	//Szukamy "GRIB" w pliku
	while (!grib.eof())
	{
		grib >> byte;
		//Jeżeli znalazłem G
		if (byte == 'G')
		{
			//Wczytaj kolejne 3 bajty
			grib >> byte2 >> byte3 >> byte4;
			if (byte2 == 'R' && byte3 == 'I' && byte4 == 'B')
			{
				//I jeżeli są rowne RIB to znalazłem słowo GRIB
				outputBuffer << "Znalazlem GRIB na pozycji: " << (uint32_t)grib.tellg() - 4 << "\n";
				//Start sekcji 0 to aktualna pozycja (tellg)  - 4 bajty ( dlugosc słowa GRIB)
				start_section_0 = (uint32_t)grib.tellg() - 4;
				break;
			}
		}
	}

	//Szukamy 7777 oznaczajacego koniec pliku
	uint32_t start_7777_pos = 0;
	while (!grib.eof())
	{
		grib >> byte;
		if (byte == '7')
		{
			grib >> byte2 >> byte3 >> byte4;
			if (byte2 == '7' && byte3 == '7' && byte4 == '7')
			{
				start_7777_pos = (uint32_t)grib.tellg() - 4;
				outputBuffer << "Znalazlem 7777 na pozycji: " << (uint32_t)grib.tellg() << "\n";
				break;
			}
		}
	}

	uint32_t end_grib_pos = start_7777_pos + 4; // End of GRIB message
	uint32_t distance = end_grib_pos - start_section_0;
	outputBuffer << "Distance between end of GRIB and start of 7777: " << distance << "\n";

	//Seekg - Przesun kursor na zadaną pozycje w pliku
	grib.seekg(start_section_0 + 4);
	/*outputBuffer << "Current pos: " << grib.tellg() << "\n";*/
	uint32_t message_len = 0;

	//Wczytujemy kolejno trzy bajty i umieszczamy je w 4 bajtowej zmiennej
	for (int i = 0; i < 3; ++i)
	{
		uint8_t tmp;
		grib >> tmp;

		message_len |= (static_cast<uint32_t>(tmp) << (8 * (2 - i)));
	}

	outputBuffer << "Dlugosc wiadomosci: " << message_len << "\n";


	//Pomijamy jeden byte
	grib >> byte;


	uint32_t start_section_1 = grib.tellg();

	//Wczytujemy kolejno trzy bajty i umieszczamy je w 4 bajtowej zmiennej
	uint32_t section_1_length = 0;
	for (int i = 0; i < 3; ++i)
	{
		uint8_t tmp;
		grib >> tmp;

		//Z odpowiednim binarnym przesunięciem o wielokrotnosc 8, co daje nam kolejno 16, 8 , 0;
		section_1_length |= (static_cast<uint32_t>(tmp) << (8 * (2 - i)));
	}
	
	outputFile << "Coding info: HUIA85\n\n";
	outputFile << "==========================\n";
	outputFile << "== General Message Info ==\n";
	outputFile << "==========================\n\n";

	outputFile << outputBuffer.str();
	cout << outputBuffer.str();
	outputBuffer.str("");
	outputBuffer.clear();

	outputBuffer << "Dlugosc sekcji 1 : " << section_1_length << "\n";

	grib.seekg(start_section_1 + 3);

	uint32_t ParametertableVersion = 0;

	uint8_t tmp;
	grib >> tmp;

	ParametertableVersion |= (static_cast<uint32_t>(tmp));
	outputBuffer << "Parameter table Version: " << ParametertableVersion << "\n";

	grib.seekg(start_section_1 + 4);
	uint32_t Centre = 0;
	grib >> tmp;
	Centre |= (static_cast<uint32_t>(tmp));
	std::string nameCentre;
	if (Centre == 7)
	{
		nameCentre = "US National Weather Service - NCEP(WMC)";
	}
	else
	{
		nameCentre = "404 not found";
	}
	outputBuffer << "Identification of Centre: " << nameCentre << "\n";

	grib.seekg(start_section_1 + 5);
	uint32_t ProcessID = 0;
	grib >> tmp;
	ProcessID |= (static_cast<uint32_t>(tmp));
	std::string nameidprocess;
	if (ProcessID == 81)
	{
		nameidprocess = "Analysis from GFS (Global Forecast System)";
	}
	else
	{
		nameidprocess = "404 not found";
	}
	outputBuffer << "process ID number: " << nameidprocess << "\n";

	grib.seekg(start_section_1 + 6);
	uint32_t GridIdentification = 0;
	grib >> tmp;
	GridIdentification |= (static_cast<uint32_t>(tmp));
	outputBuffer << "Grid Identification: " << GridIdentification << "\n";

	grib.seekg(start_section_1 + 7);

	uint16_t GDSandBMS = 0;

	char byte_chars[2];
	grib.read(byte_chars, 2);

	GDSandBMS = static_cast<uint16_t>(byte_chars[0]) << 8 | static_cast<uint16_t>(byte_chars[1]);

	bool isGDSIncluded = (GDSandBMS & 0b00000001) != 0;
	bool isBMSIncluded = (GDSandBMS & 0b00000010) != 0;

	string GDS, BMS;

	if (isGDSIncluded)
	{
		GDS = "GDS Included";
	}
	else
	{
		GDS = "GDS Omitted";
	}

	if (isBMSIncluded)
	{
		BMS = "BMS Included";
	}
	else
	{
		BMS = "BMS Omitted";
	}

	outputBuffer << "GDS and BMS: " << GDS << " " << BMS << "\n";


	grib.seekg(start_section_1 + 8);
	uint32_t Unitparameters = 0;
	grib >> tmp;
	Unitparameters |= (static_cast<uint32_t>(tmp));
	outputBuffer << "Unit parameters: " << Unitparameters << "\n";

	grib.seekg(start_section_1 + 9);
	uint32_t Indicatoroftypeoflevelorlayer = 0;
	grib >> tmp;
	Indicatoroftypeoflevelorlayer |= (static_cast<uint32_t>(tmp));
	outputBuffer << "Indicator of type of level or layer: " << Indicatoroftypeoflevelorlayer << "\n";

	grib.seekg(start_section_1 + 10);
	uint32_t Heightpressure = 0;
	grib >> tmp;
	Heightpressure |= (static_cast<uint32_t>(tmp));
	outputBuffer << "Height, pressure, etc. of the level or layer: " << Heightpressure << "\n";

	grib.seekg(start_section_1 + 12);

	// Read Year
	grib.read(&byte_char, 1);
	int Year = int(byte_char);

	// Read Month
	grib.read(&byte_char, 1);
	int Month = int(byte_char);

	// Read Day
	grib.read(&byte_char, 1);
	int Day = int(byte_char);

	// Read Hour
	grib.read(&byte_char, 1);
	int hours = int(byte_char);

	// Read Minute
	grib.read(&byte_char, 1);
	int minute = int(byte_char);

	// Output date and time
	outputBuffer << "YY/MM/DD|HH:MM: " << Year << "/" << Month << "/" << Day << "|" << hours << ":" << minute << "\n";






	outputFile << "==========================\n";
	outputFile << "== Section 1 Data ==\n";
	outputFile << "==========================\n\n";

	outputFile << outputBuffer.str();

	cout << outputBuffer.str();

	return 0;
}