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


	while (!grib.eof())
	{
		grib >> byte;
		if (byte == 'G')
		{
			grib >> byte2 >> byte3 >> byte4;
			if (byte2 == 'R' && byte3 == 'I' && byte4 == 'B')
			{
				outputBuffer << "Znalazlem GRIB na pozycji: " << (uint32_t)grib.tellg() - 4 << "\n";
				start_section_0 = (uint32_t)grib.tellg() - 4;
				break;
			}
		}
	}

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

	grib.seekg(start_section_0 + 4);

	uint32_t message_len = 0;

	for (int i = 0; i < 3; ++i)
	{
		uint8_t tmp;
		grib >> tmp;

		message_len |= (static_cast<uint32_t>(tmp) << (8 * (2 - i)));
	}

	outputBuffer << "Dlugosc wiadomosci: " << message_len << "\n";


	grib >> byte;


	uint32_t start_section_1 = grib.tellg();

	uint32_t section_1_length = 0;
	for (int i = 0; i < 3; ++i)
	{
		uint8_t tmp;
		grib >> tmp;

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
	grib.read(&byte_char, 1);
	int Heightpressure = int(byte_char);
	grib.read(&byte_char, 1);
	int Highest = int(byte_char);

	outputBuffer << "Height, pressure, etc. of the level or layer: " << Heightpressure << Highest <<"\n";

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
	outputBuffer << "YY/MM/DD|HH:MM: "
	<< Year << "/"
	<< Month << "/"
	<< Day << "|"
	<< (hours < 10 ? "0" : "") << hours << ":"
	<< (minute < 10 ? "0" : "") << minute << "\n";

	grib.seekg(start_section_1 + 17);
	grib.read(&byte_char, 1);
	int TimeUnit = int(byte_char);
	outputBuffer << "Time unit: " << TimeUnit << "\n";

	grib.seekg(start_section_1 + 18);
	grib.read(&byte_char, 1);
	int P1 = int(byte_char);
	outputBuffer << "P1 - Period of time: " << P1 << "\n";

	grib.seekg(start_section_1 + 19);
	grib.read(&byte_char, 1);
	int P2 = int(byte_char);
	outputBuffer << "P2 - Period of time: " << P2 << "\n";

	grib.seekg(start_section_1 + 20);
	grib.read(&byte_char, 1);
	int TimeRange = int(byte_char);
	outputBuffer << "Time range indicator: " << TimeRange << "\n";

	grib.seekg(start_section_1 + 21);
	grib.read(&byte_char, 1);
	int LastRow = int(byte_char);
	outputBuffer << "Numbers for last row: " << LastRow << "\n";

	grib.seekg(start_section_1 + 23);
	grib.read(&byte_char, 1);
	int MissingAverages = int(byte_char);
	outputBuffer << "Number missing from averages or accumulation: " << MissingAverages << "\n";

	grib.seekg(start_section_1 + 24);
	grib.read(&byte_char, 1);
	int Referencecentury = int(byte_char);
	outputBuffer << "Reference century: " << Referencecentury << "\n";

	grib.seekg(start_section_1 + 25);
	grib.read(&byte_char, 1);
	int subCenter = int(byte_char);
	outputBuffer << "Identification of sub Center: " << subCenter << "\n";

	grib.seekg(start_section_1 + 26);
	grib.read(&byte_char, 1);
	int DecimalScalefactor = int(byte_char);
	grib.read(&byte_char, 1);
	int DecimalScalefactor2 = int(byte_char);
	outputBuffer << "Decimal Scale factor: " << DecimalScalefactor2 << "\n";



	outputFile << "==========================\n";
	outputFile << "== Section 1 Data ==\n";
	outputFile << "==========================\n\n";

	outputFile << outputBuffer.str();

	cout << outputBuffer.str();
	outputBuffer.str("");
	outputBuffer.clear();

	uint32_t start_section_2 = grib.tellg();

	uint32_t section_2_length = 0;
	for (int i = 0; i < 3; ++i)
	{
		uint8_t tmp;
		grib >> tmp;

		section_2_length |= (static_cast<uint32_t>(tmp) << (8 * (2 - i)));
	}
	outputBuffer << "Dlugosc sekcji 2 : " << section_2_length << "\n";

	grib.seekg(start_section_2 + 3);
	grib.read(&byte_char, 1);
	int NumberofVerticalCoordinates = int(byte_char);
	outputBuffer << "Number of Vertical Coordinates: " << NumberofVerticalCoordinates << "\n";

	grib.seekg(start_section_2 + 4);
	grib.read(&byte_char, 1);
	int Octetnumber = int(byte_char);
	outputBuffer << "Octet number: " << Octetnumber << "\n";

	grib.seekg(start_section_2 + 5);
	grib.read(&byte_char, 1);
	int Datarepresentationtype = int(byte_char);
	outputBuffer << "Data representation type: " << Datarepresentationtype << "\n";

	grib.seekg(start_section_2 + 6);
	char byte_charso[2];
	grib.read(byte_charso, 2);
	int numberOfPoints = static_cast<int>(static_cast<unsigned char>(byte_charso[0])) << 8 | static_cast<int>(static_cast<unsigned char>(byte_charso[1]));

	outputBuffer << "Numbers of points along a latitude circle: " << numberOfPoints << "\n";

	grib.seekg(start_section_2 + 8);
	char byte_chars2[2];
	grib.read(byte_chars2, 2);
	int numberOfPointsMeridian = static_cast<int>(static_cast<unsigned char>(byte_chars2[0])) << 8 | static_cast<int>(static_cast<unsigned char>(byte_chars2[1]));

	outputBuffer << "Numbers of points along a longitude meridian: " << numberOfPointsMeridian << "\n";

	grib.seekg(start_section_2 + 10);
	char byte_chars3[2];
	grib.read(byte_chars3, 2);
	int Latitude = static_cast<int>(static_cast<unsigned char>(byte_chars3[0])) << 8 | static_cast<int>(static_cast<unsigned char>(byte_chars3[1]));

	outputBuffer << "Latitude: " << Latitude << " degrees" << "\n";

	grib.seekg(start_section_2 + 13);
	char byte_chars4[3];
	grib.read(byte_chars4, 3);
	int Lo1_millidegrees = (static_cast<int>(static_cast<unsigned char>(byte_chars[0])) << 16) | (static_cast<int>(static_cast<unsigned char>(byte_chars[1])) << 8) | static_cast<int>(static_cast<unsigned char>(byte_chars[2]));

	bool isWestLongitude = (Lo1_millidegrees & 0x800000) != 0;

	Lo1_millidegrees &= 0x7FFFFF;

	double Lo1_degrees = static_cast<double>(Lo1_millidegrees) / 1000.0;

	outputBuffer << "Longitude: ";
	if (isWestLongitude) {
		outputBuffer << Lo1_degrees << "° W\n";
	}
	else {
		outputBuffer << Lo1_degrees << "° E\n";
	}

	grib.seekg(start_section_2 + 16);

	char resolution_component_flags;
	grib.read(&resolution_component_flags, 1);

	bool direction_increments_given = (resolution_component_flags & 0b00000001) != 0;
	bool earth_oblate_spheroid = (resolution_component_flags & 0b00000010) != 0;
	bool u_v_components_relative_defined_grid = (resolution_component_flags & 0b00010000) != 0;

	outputBuffer << "Resolution and component flags:\n";
	outputBuffer << "Direction increments given: " << (direction_increments_given ? "Yes" : "No") << "\n";
	outputBuffer << "Earth shape: " << (earth_oblate_spheroid ? "Oblate spheroid" : "Spherical") << "\n";
	outputBuffer << "Components resolved relative to grid: " << (u_v_components_relative_defined_grid ? "Yes" : "No") << "\n";

	grib.seekg(start_section_2 + 17);

	char byte_chars6[3];
	grib.read(byte_chars6, 3);

	int La2_millidegrees = (static_cast<int>(static_cast<unsigned char>(byte_chars6[0])) << 16) | (static_cast<int>(static_cast<unsigned char>(byte_chars6[1])) << 8) | static_cast<int>(static_cast<unsigned char>(byte_chars6[2]));

	bool isSouthLatitude = (La2_millidegrees & 0x800000) != 0;

	La2_millidegrees &= 0x7FFFFF;

	double La2_degrees = static_cast<double>(La2_millidegrees) / 1000.0;

	outputBuffer << "Latitude of last grid point: " << La2_degrees << " degrees\n";

	grib.seekg(start_section_2 + 20);

	char byte_chars7[3];
	grib.read(byte_chars7, 3);

	int La3_millidegrees = (static_cast<int>(static_cast<unsigned char>(byte_chars7[0])) << 16) | (static_cast<int>(static_cast<unsigned char>(byte_chars7[1])) << 8) | static_cast<int>(static_cast<unsigned char>(byte_chars7[2]));

	bool isSouthLatitude2 = (La3_millidegrees & 0x800000) != 0;

	La3_millidegrees &= 0x7FFFFF;

	double La3_degrees = static_cast<double>(La3_millidegrees) / 1000.0;

	outputBuffer << "Longitude of last grid point: " << La3_degrees << " degrees\n";

	
	grib.seekg(start_section_2 + 23);
	grib.read(&byte_char, 1);
	int DirectionIncrement = int(byte_char);
	outputBuffer << "Longitudinal Direction Increment: " << DirectionIncrement << "\n";

	grib.seekg(start_section_2 + 25);
	char byte_chars9[2];
	grib.read(byte_chars9, 2);
	int LatitudinalDirection = static_cast<int>(static_cast<unsigned char>(byte_chars9[0])) << 8 | static_cast<int>(static_cast<unsigned char>(byte_chars9[1]));

	outputBuffer << "Latitudinal Direction Increment: " << LatitudinalDirection <<  "\n";

	


	outputFile << "==========================\n";
	outputFile << "== Section 2 Data ==\n";
	outputFile << "==========================\n\n";

	outputFile << outputBuffer.str();
	cout << outputBuffer.str();
	outputBuffer.str("");
	outputBuffer.clear();


	// Przesuń kursor na koniec sekcji 3
	grib.seekg(start_section_2 + section_2_length);

	uint32_t start_section_3 = grib.tellg();

	//Wczytujemy kolejno trzy bajty i umieszczamy je w 4 bajtowej zmiennej
	uint32_t section_3_length = 0;
	for (int i = 0; i < 3; ++i)
	{
		uint8_t tmp;
		grib >> tmp;

		//Z odpowiednim binarnym przesunięciem o wielokrotnosc 8, co daje nam kolejno 16, 8 , 0;
		section_3_length |= (static_cast<uint32_t>(tmp) << (8 * (2 - i)));
	}
	outputBuffer << "Dlugosc sekcji 3 : " << section_3_length << "\n";

	grib.read(&byte_char, 1);
	int Flag_to_decode = int(byte_char);
	outputBuffer << "Flag to decode: " << Flag_to_decode << "\n";

	grib.read(&byte_char, 1);
	int BinaryScaleFactor = int(byte_char);
	outputBuffer << "Binary Scale Factor: : " << BinaryScaleFactor << "\n";

	outputFile << "==========================\n";
	outputFile << "== Section 3 Data ==\n";
	outputFile << "==========================\n\n";

	outputFile << outputBuffer.str();
	cout << outputBuffer.str();

	return 0;
}