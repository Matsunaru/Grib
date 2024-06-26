#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <bitset>
#include <vector>
using namespace std;

int main()
{
    fstream grib;
    // Open the file in read and binary mode
    grib.open("D:\\StudiaCC\\StudiaCC\\all.grib", ios::in | ios::binary);

    if (!grib.is_open())
    {
        cout << "Failed to open the file!\n";
        return -1;
    }

    ofstream outputFile("output.txt");

    if (!outputFile.is_open())
    {
        cout << "Failed to open the output file!\n";
        return -1;
    }

    stringstream outputBuffer;

    uint32_t start_section_0 = 0;

    uint8_t byte, byte2, byte3, byte4;
    char byte_char;

    // Search for "GRIB" in the file
    while (!grib.eof())
    {
        grib.read(reinterpret_cast<char*>(&byte), 1);
        if (byte == 'G')
        {
            grib.read(reinterpret_cast<char*>(&byte2), 1);
            grib.read(reinterpret_cast<char*>(&byte3), 1);
            grib.read(reinterpret_cast<char*>(&byte4), 1);
            if (byte2 == 'R' && byte3 == 'I' && byte4 == 'B')
            {
                outputBuffer << "Found GRIB at position: " << (uint32_t)grib.tellg() - 4 << "\n";
                start_section_0 = (uint32_t)grib.tellg() - 4;
                break;
            }
        }
    }

    // Search for 7777 indicating the end of the file
    uint32_t start_7777_pos = 0;
    while (!grib.eof())
    {
        grib.read(reinterpret_cast<char*>(&byte), 1);
        if (byte == '7')
        {
            grib.read(reinterpret_cast<char*>(&byte2), 1);
            grib.read(reinterpret_cast<char*>(&byte3), 1);
            grib.read(reinterpret_cast<char*>(&byte4), 1);
            if (byte2 == '7' && byte3 == '7' && byte4 == '7')
            {
                start_7777_pos = (uint32_t)grib.tellg() - 4;
                outputBuffer << "Found 7777 at position: " << (uint32_t)grib.tellg() << "\n";
                break;
            }
        }
    }

    uint32_t end_grib_pos = start_7777_pos + 4; // End of GRIB message
    uint32_t distance = end_grib_pos - start_section_0;
    outputBuffer << "Distance between end of GRIB and start of 7777: " << distance << "\n";

    grib.seekg(start_section_0 + 4);
    uint32_t message_len = 0;

    // Read the next three bytes into a 4-byte variable
    for (int i = 0; i < 3; ++i)
    {
        grib.read(reinterpret_cast<char*>(&byte), 1);
        message_len |= (static_cast<uint32_t>(byte) << (8 * (2 - i)));
    }

    outputBuffer << "Message length: " << message_len << "\n";

    // Skip one byte
    grib.read(reinterpret_cast<char*>(&byte), 1);

    uint32_t start_section_1 = grib.tellg();

    // Read the next three bytes into a 4-byte variable
    uint32_t section_1_length = 0;
    for (int i = 0; i < 3; ++i)
    {
        grib.read(reinterpret_cast<char*>(&byte), 1);
        section_1_length |= (static_cast<uint32_t>(byte) << (8 * (2 - i)));
    }

    outputFile << "Coding info: HUIA85\n\n";
    outputFile << "==========================\n";
    outputFile << "== General Message Info ==\n";
    outputFile << "==========================\n\n";

    outputFile << outputBuffer.str();
    cout << outputBuffer.str();
    outputBuffer.str("");
    outputBuffer.clear();

    outputBuffer << "Section 1 length: " << section_1_length << "\n";

    
    uint32_t ParametertableVersion = 0;
    grib.read(reinterpret_cast<char*>(&byte), 1);
    ParametertableVersion |= (static_cast<uint32_t>(byte));
    outputBuffer << "Parameter table Version: " << ParametertableVersion << "\n";

    
    uint32_t Centre = 0;
    grib.read(reinterpret_cast<char*>(&byte), 1);
    Centre |= (static_cast<uint32_t>(byte));
    string nameCentre = (Centre == 7) ? "US National Weather Service - NCEP(WMC)" : "404 not found";
    outputBuffer << "Identification of Centre: " << nameCentre << "\n";

    
    uint32_t ProcessID = 0;
    grib.read(reinterpret_cast<char*>(&byte), 1);
    ProcessID |= (static_cast<uint32_t>(byte));
    string nameidprocess = (ProcessID == 81) ? "Analysis from GFS (Global Forecast System)" : "404 not found";
    outputBuffer << "Process ID number: " << nameidprocess << "\n";

    uint32_t GridIdentification = 0;
    grib.read(reinterpret_cast<char*>(&byte), 1);
    GridIdentification |= (static_cast<uint32_t>(byte));
    outputBuffer << "Grid Identification: " << GridIdentification << "\n";

    uint16_t GDSandBMS = 0;
    char byte_chars[2];
    grib.read(byte_chars, 2);

    GDSandBMS = static_cast<uint16_t>(static_cast<unsigned char>(byte_chars[0])) << 8 |
        static_cast<uint16_t>(static_cast<unsigned char>(byte_chars[1]));

    string binary = bitset<16>(GDSandBMS).to_string();
    string output_GDS_BMS;

    output_GDS_BMS += (binary[binary.size() - 1] == '1') ? "GDS Included " : "GDS Omitted ";
    output_GDS_BMS += (binary[binary.size() - 2] == '1') ? "BMS Included " : "BMS Omitted ";

    bool reserved = false;
    for (size_t i = 0; i < binary.size() - 2; ++i)
    {
        if (binary[i] == '1')
        {
            reserved = true;
            break;
        }
    }

    if (reserved) {
        output_GDS_BMS += " reserved";
    }

    outputBuffer << "GDS and BMS: " << output_GDS_BMS << "\n";

    grib.seekg(start_section_1 + 8);
    uint32_t Unitparameters = 0;
    grib.read(&byte_char, 1);
    Unitparameters |= (static_cast<uint32_t>(byte_char));
    outputBuffer << "Unit parameters: " << Unitparameters << "\n";

    uint32_t Indicatoroftypeoflevelorlayer = 0;
    grib.read(reinterpret_cast<char*>(&byte), 1);
    Indicatoroftypeoflevelorlayer |= (static_cast<uint32_t>(byte));
    outputBuffer << "Indicator of type of level or layer: " << Indicatoroftypeoflevelorlayer << "\n";

    // Read height, pressure, etc. of the level or layer
    grib.read(&byte_char, 1);
    int firstByte = static_cast<unsigned char>(byte_char);
    grib.read(&byte_char, 1);
    int secondByte = static_cast<unsigned char>(byte_char);
    int Heightpressure = (firstByte << 8) | secondByte;
    outputBuffer << "Height, pressure, etc. of the level or layer: " << Heightpressure << "\n";


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
        << setw(2) << setfill('0') << hours << ":"
        << setw(2) << setfill('0') << minute << "\n";

    grib.read(&byte_char, 1);
    int TimeUnit = int(byte_char);
    outputBuffer << "Time unit: " << TimeUnit << "\n";

    grib.read(&byte_char, 1);
    int P1 = int(byte_char);
    outputBuffer << "P1 - Period of time: " << P1 << "\n";

    grib.read(&byte_char, 1);
    int P2 = int(byte_char);
    outputBuffer << "P2 - Period of time: " << P2 << "\n";

    grib.read(&byte_char, 1);
    int TimeRange = int(byte_char);
    outputBuffer << "Time range indicator: " << TimeRange << "\n";

    grib.read(&byte_char, 1);
    int LastRow = int(byte_char);
    outputBuffer << "Numbers for last row: " << LastRow << "\n";

    grib.seekg(start_section_1 + 23);
    grib.read(&byte_char, 1);
    int MissingAverages = int(byte_char);
    outputBuffer << "Number missing from averages or accumulation: " << MissingAverages << "\n";

    grib.read(&byte_char, 1);
    int Referencecentury = int(byte_char);
    outputBuffer << "Reference century: " << Referencecentury << "\n";

    grib.read(&byte_char, 1);
    int subCenter = int(byte_char);
    outputBuffer << "Identification of sub Center: " << subCenter << "\n";

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

    // Read the next three bytes into a 4-byte variable
    uint32_t section_2_length = 0;
    for (int i = 0; i < 3; ++i)
    {
        grib.read(reinterpret_cast<char*>(&byte), 1);
        section_2_length |= (static_cast<uint32_t>(byte) << (8 * (2 - i)));
    }
    outputBuffer << "Section 2 length: " << section_2_length << "\n";

    grib.read(&byte_char, 1);
    int NumberofVerticalCoordinates = int(byte_char);
    outputBuffer << "Number of Vertical Coordinates: " << NumberofVerticalCoordinates << "\n";

    grib.read(&byte_char, 1);
    int Octetnumber = int(byte_char);
    outputBuffer << "Octet number: " << Octetnumber << "\n";


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
    int Lo1_millidegrees = (static_cast<int>(static_cast<unsigned char>(byte_chars4[0])) << 16) |
        (static_cast<int>(static_cast<unsigned char>(byte_chars4[1])) << 8) |
        static_cast<int>(static_cast<unsigned char>(byte_chars4[2]));

    bool isWestLongitude = (Lo1_millidegrees & 0x800000) != 0;

    // Remove the most significant bit as it is used to denote west longitude
    Lo1_millidegrees &= 0x7FFFFF;

    // Convert millidegrees to degrees
    double Lo1_degrees = static_cast<double>(Lo1_millidegrees) / 1000.0;

    if (isWestLongitude) {
        Lo1_degrees = -Lo1_degrees;
    }
    // Add longitude information to the buffer
    outputBuffer << "Longitude: " << Lo1_degrees << " degrees\n";

    // Move to the position for resolution and component flags
    grib.seekg(start_section_2 + 16);
    char resolution_component_flags;
    grib.read(&resolution_component_flags, 1);

    // Read each flag
    bool direction_increments_given = (resolution_component_flags & 0b00000001) != 0;
    bool earth_oblate_spheroid = (resolution_component_flags & 0b00000010) != 0;
    bool u_v_components_relative_defined_grid = (resolution_component_flags & 0b00010000) != 0;
    bool reserved_3_4 = (resolution_component_flags & 0b00001100) != 0;
    bool reserved_6_8 = (resolution_component_flags & 0b11100000) != 0;

    // Add flag information to the buffer
    outputBuffer << "Resolution and component flags:\n";
    outputBuffer << "Direction increments given: " << (direction_increments_given ? "Direction increments given" : "Direction increments not given") << "\n";
    outputBuffer << "Earth shape: " << (earth_oblate_spheroid ? "Earth assumed oblate spheroid with size as determined by IAU in 1965: 6378.160 km, 6356.775 km, f = 1/297.0" : "Earth assumed spherical with radius = 6367.47 km") << "\n";
    outputBuffer << "Reserved (3-4): " << (reserved_3_4 ? "Set to 1" : "Reserved (set to 0)") << "\n";
    outputBuffer << "u- and v-components of vector quantities: " << (u_v_components_relative_defined_grid ? "u- and v-components of vector quantities resolved relative to the defined grid in the direction of increasing x and y (or i and j) coordinates respectively" : "u- and v-components of vector quantities resolved relative to easterly and northerly directions") << "\n";
    outputBuffer << "Reserved (6-8): " << (reserved_6_8 ? "Set to 1" : "Reserved (set to 0)") << "\n";

    // Move to the position for latitude of last grid point
    grib.seekg(start_section_2 + 17);
    char byte_chars6[3];
    grib.read(byte_chars6, 3);

    int La2_millidegrees = (static_cast<int>(static_cast<unsigned char>(byte_chars6[0])) << 16) |
        (static_cast<int>(static_cast<unsigned char>(byte_chars6[1])) << 8) |
        static_cast<int>(static_cast<unsigned char>(byte_chars6[2]));

    bool isSouthLatitude = (La2_millidegrees & 0x800000) != 0;
    La2_millidegrees &= 0x7FFFFF;
    double La2_degrees = static_cast<double>(La2_millidegrees) / 1000.0;

    outputBuffer << "Latitude of last grid point: " << La2_degrees << " degrees\n";

    // Move to the position for longitude of last grid point
    grib.seekg(start_section_2 + 20);
    char byte_chars7[3];
    grib.read(byte_chars7, 3);

    int Lo2_millidegrees = (static_cast<int>(static_cast<unsigned char>(byte_chars7[0])) << 16) |
        (static_cast<int>(static_cast<unsigned char>(byte_chars7[1])) << 8) |
        static_cast<int>(static_cast<unsigned char>(byte_chars7[2]));

    bool isSouthLongitude = (Lo2_millidegrees & 0x800000) != 0;
    Lo2_millidegrees &= 0x7FFFFF;
    double Lo2_degrees = static_cast<double>(Lo2_millidegrees) / 1000.0;

    outputBuffer << "Longitude of last grid point: " << Lo2_degrees << " degrees\n";

    // Move to the position for longitudinal direction increment
    grib.seekg(start_section_2 + 23);

    // Read two bytes
    char byte_chars8[2];
    grib.read(byte_chars8, 2);

    // Convert two bytes to a 16-bit integer
    int DirectionIncrement = (static_cast<int>(static_cast<unsigned char>(byte_chars8[0])) << 8) |
        static_cast<int>(static_cast<unsigned char>(byte_chars8[1]));

    // Convert value to degrees (dividing by 1000)
    double DirectionIncrementDegrees = static_cast<double>(DirectionIncrement) / 1000.0;

    // Add result to the buffer
    outputBuffer << "Longitudinal Direction Increment: " << DirectionIncrementDegrees << " degrees\n";

    // Move to the position for latitudinal direction increment
    grib.seekg(start_section_2 + 25);

    // Read two bytes
    char byte_chars9[2];
    grib.read(byte_chars9, 2);

    // Convert two bytes to a 16-bit integer
    int LatitudinalDirection = (static_cast<int>(static_cast<unsigned char>(byte_chars9[0])) << 8) |
        static_cast<int>(static_cast<unsigned char>(byte_chars9[1]));

    // Convert value to degrees (dividing by 1000)
    double LatitudinalDirectionDegrees = static_cast<double>(LatitudinalDirection) / 1000.0;

    // Add result to the buffer
    outputBuffer << "Latitudinal Direction Increment: " << LatitudinalDirectionDegrees << " degrees\n";

    // Read scanning mode flags
    grib.seekg(start_section_2 + 27);  // Octet 28 (0-based index is 27)
    char scanning_mode_flags;
    grib.read(&scanning_mode_flags, 1);

    bool scan_i_direction = (scanning_mode_flags & 0b00000001) != 0;
    bool scan_j_direction = (scanning_mode_flags & 0b00000010) != 0;
    bool adj_points_i_direction = (scanning_mode_flags & 0b00000100) != 0;
    bool adj_points_j_direction = (scanning_mode_flags & 0b00001000) != 0;

    outputBuffer << "Scanning mode flags: Points scan in "
        << (scan_i_direction ? "-i" : "+i")
        << " and "
        << (scan_j_direction ? "+j" : "-j")
        << " direction: (FORTRAN: ("
        << (adj_points_i_direction ? "J,I" : "I,J")
        << "))\n";

    // Read the number of points in each row
    int num_rows = 73;  // Based on the provided example
    grib.seekg(start_section_2 + 32);  // Octet 33 (0-based index is 32)
    vector<int> row_points(num_rows);

    for (int i = 0; i < num_rows; ++i) {
        char bytes[2];
        grib.read(bytes, 2);
        row_points[i] = (static_cast<int>(static_cast<unsigned char>(bytes[0])) << 8) |
            static_cast<int>(static_cast<unsigned char>(bytes[1]));
    }

    for (int i = 0; i < num_rows; ++i) {
        outputBuffer << "Row " << (i + 1) << " Points: " << row_points[i] << "\n";
    }

    outputFile << "==========================\n";
    outputFile << "== Section 2 Data ==\n";
    outputFile << "==========================\n\n";

    outputFile << outputBuffer.str();
    cout << outputBuffer.str();
    outputBuffer.str("");
    outputBuffer.clear();

    // Move to the end of section 2
    grib.seekg(start_section_2 + section_2_length);

    uint32_t start_section_3 = grib.tellg();

    // Read the next three bytes into a 4-byte variable
    uint32_t section_3_length = 0;
    for (int i = 0; i < 3; ++i)
    {
        grib.read(reinterpret_cast<char*>(&byte), 1);
        section_3_length |= (static_cast<uint32_t>(byte) << (8 * (2 - i)));
    }
    outputBuffer << "Section 3 length: " << section_3_length << "\n";

    grib.read(&byte_char, 1);
    int Flag_to_decode = int(byte_char);
    outputBuffer << "Flag to decode: " << Flag_to_decode << "\n";

    grib.read(&byte_char, 1);
    int BinaryScaleFactor = int(byte_char);
    outputBuffer << "Binary Scale Factor: " << BinaryScaleFactor << "\n";

    outputFile << "==========================\n";
    outputFile << "== Section 3 Data ==\n";
    outputFile << "==========================\n\n";

    outputFile << outputBuffer.str();
    cout << outputBuffer.str();

    grib.close();
    outputFile.close();

    return 0;
}