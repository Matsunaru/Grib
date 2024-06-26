#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <bitset>
#include <vector>
using namespace std;

// Function to read a specified number of bytes and convert to an integer
uint32_t readBytesAsInt(fstream &file, int numBytes)
{
    uint32_t value = 0;
    for (int i = 0; i < numBytes; ++i)
    {
        char byte;
        file.read(&byte, 1);
        value |= (static_cast<uint32_t>(byte) & 0xFF) << (8 * (numBytes - 1 - i));
    }
    return value;
}

int main()
{
    fstream grib("D:\\StudiaCC\\StudiaCC\\all.grib", ios::in | ios::binary);
    if (!grib.is_open())
    {
        cerr << "Failed to open the file!\n";
        return -1;
    }

    ofstream outputFile("output.txt");
    if (!outputFile.is_open())
    {
        cerr << "Failed to open the output file!\n";
        return -1;
    }

    stringstream outputBuffer;

    // Search for "GRIB" in the file
    uint32_t start_section_0 = 0;
    while (!grib.eof())
    {
        if (grib.get() == 'G' && grib.get() == 'R' && grib.get() == 'I' && grib.get() == 'B')
        {
            start_section_0 = static_cast<uint32_t>(grib.tellg()) - 4;
            outputBuffer << "Found GRIB at position: " << start_section_0 << "\n";
            break;
        }
    }

    // Search for 7777 indicating the end of the file
    uint32_t start_7777_pos = 0;
    while (!grib.eof())
    {
        if (grib.get() == '7' && grib.get() == '7' && grib.get() == '7' && grib.get() == '7')
        {
            start_7777_pos = static_cast<uint32_t>(grib.tellg()) - 4;
            outputBuffer << "Found 7777 at position: " << start_7777_pos + 4 << "\n";
            break;
        }
    }

    uint32_t end_grib_pos = start_7777_pos + 4;
    uint32_t distance = end_grib_pos - start_section_0;
    outputBuffer << "Distance between end of GRIB and start of 7777: " << distance << "\n";

    // Read the message length (next 3 bytes)
    grib.seekg(start_section_0 + 4);
    uint32_t message_len = readBytesAsInt(grib, 3);
    outputBuffer << "Message length: " << message_len << "\n";

    // Skip one byte
    grib.ignore(1);


    outputFile << "Coding info: HUIA85\n\n";
    outputFile << "==========================\n";
    outputFile << "== General Message Info ==\n";
    outputFile << "==========================\n\n";

    outputFile << outputBuffer.str();
    cout << outputBuffer.str();
    outputBuffer.str("");
    outputBuffer.clear();

    uint32_t start_section_1 = grib.tellg();
    uint32_t section_1_length = readBytesAsInt(grib, 3);
    outputBuffer << "Section 1 length: " << section_1_length << "\n";

    uint32_t ParametertableVersion = readBytesAsInt(grib, 1);
    outputBuffer << "Parameter table Version: " << ParametertableVersion << "\n";

    uint32_t Centre = readBytesAsInt(grib, 1);
    string nameCentre = (Centre == 7) ? "US National Weather Service - NCEP(WMC)" : "404 not found";
    outputBuffer << "Identification of Centre: " << nameCentre << "\n";

    uint32_t ProcessID = readBytesAsInt(grib, 1);
    string nameidprocess = (ProcessID == 81) ? "Analysis from GFS (Global Forecast System)" : "404 not found";
    outputBuffer << "Process ID number: " << nameidprocess << "\n";

    uint32_t GridIdentification = readBytesAsInt(grib, 1);
    outputBuffer << "Grid Identification: " << GridIdentification << "\n";

    uint8_t GDSandBMS = readBytesAsInt(grib, 1);
    string binary = bitset<8>(GDSandBMS).to_string();
    string output_GDS_BMS = (binary[binary.size() - 1] == '1') ? "GDS Included " : "GDS Omitted ";
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

    if (reserved)
    {
        output_GDS_BMS += " reserved";
    }

    outputBuffer << "GDS and BMS: " << output_GDS_BMS << "\n";

    uint32_t Unitparameters = readBytesAsInt(grib, 1);
    outputBuffer << "Unit parameters: " << Unitparameters << "\n";

    uint32_t Indicatoroftypeoflevelorlayer = readBytesAsInt(grib, 1);
    outputBuffer << "Indicator of type of level or layer: " << Indicatoroftypeoflevelorlayer << "\n";

    // Read height, pressure, etc. of the level or layer
    uint32_t Heightpressure = readBytesAsInt(grib, 2);
    outputBuffer << "Height, pressure, etc. of the level or layer: " << Heightpressure << "\n";

    // Read date and time
    int Year = readBytesAsInt(grib, 1);
    int Month = readBytesAsInt(grib, 1);
    int Day = readBytesAsInt(grib, 1);
    int hours = readBytesAsInt(grib, 1);
    int minute = readBytesAsInt(grib, 1);

    // Output date and time
    outputBuffer << "YY/MM/DD|HH:MM: "
                 << Year << "/"
                 << Month << "/"
                 << Day << "|"
                 << setw(2) << setfill('0') << hours << ":"
                 << setw(2) << setfill('0') << minute << "\n";

    int TimeUnit = readBytesAsInt(grib, 1);
    outputBuffer << "Time unit: " << TimeUnit << "\n";

    int P1 = readBytesAsInt(grib, 1);
    outputBuffer << "P1 - Period of time: " << P1 << "\n";

    int P2 = readBytesAsInt(grib, 1);
    outputBuffer << "P2 - Period of time: " << P2 << "\n";

    int TimeRange = readBytesAsInt(grib, 1);
    outputBuffer << "Time range indicator: " << TimeRange << "\n";

    int LastRow = readBytesAsInt(grib, 2);
    outputBuffer << "Numbers for last row: " << LastRow << "\n";

    int MissingAverages = readBytesAsInt(grib, 1);
    outputBuffer << "Number missing from averages or accumulation: " << MissingAverages << "\n";

    int Referencecentury = readBytesAsInt(grib, 1);
    outputBuffer << "Reference century: " << Referencecentury << "\n";

    int subCenter = readBytesAsInt(grib, 1);
    outputBuffer << "Identification of sub Center: " << subCenter << "\n";

    int DecimalScalefactor = readBytesAsInt(grib, 1);
    int DecimalScalefactor2 = readBytesAsInt(grib, 1);
    outputBuffer << "Decimal Scale factor: " << DecimalScalefactor2 << "\n";

    outputFile << "==========================\n";
    outputFile << "== Section 1 Data ==\n";
    outputFile << "==========================\n\n";

    outputFile << outputBuffer.str();
    cout << outputBuffer.str();
    outputBuffer.str("");
    outputBuffer.clear();

    // Section 2
    grib.seekg(start_section_1 + section_1_length);
    uint32_t start_section_2 = grib.tellg();

    uint32_t section_2_length = readBytesAsInt(grib, 3);
    outputBuffer << "Section 2 length: " << section_2_length << "\n";

    int NumberofVerticalCoordinates = readBytesAsInt(grib, 1);
    outputBuffer << "Number of Vertical Coordinates: " << NumberofVerticalCoordinates << "\n";

    int Octetnumber = readBytesAsInt(grib, 1);
    outputBuffer << "Octet number: " << Octetnumber << "\n";

    int Datarepresentationtype = readBytesAsInt(grib, 1);
    outputBuffer << "Data representation type: " << Datarepresentationtype << "\n";

    int numberOfPoints = readBytesAsInt(grib, 2);
    outputBuffer << "Numbers of points along a latitude circle: " << numberOfPoints << "\n";

    int numberOfPointsMeridian = readBytesAsInt(grib, 2);
    outputBuffer << "Numbers of points along a longitude meridian: " << numberOfPointsMeridian << "\n";

    int Latitude = readBytesAsInt(grib, 3);
    outputBuffer << "Latitude: " << Latitude << " degrees" << "\n";

    // Read longitude and convert
    int Lo1_millidegrees = readBytesAsInt(grib, 3);
    bool isWestLongitude = (Lo1_millidegrees & 0x800000) != 0;
    Lo1_millidegrees &= 0x7FFFFF;
    double Lo1_degrees = static_cast<double>(Lo1_millidegrees) / 1000.0;
    if (isWestLongitude)
    {
        Lo1_degrees = -Lo1_degrees;
    }
    outputBuffer << "Longitude: " << Lo1_degrees << " degrees\n";

    // Read resolution and component flags
    char resolution_component_flags;
    grib.read(&resolution_component_flags, 1);

    bool direction_increments_given = (resolution_component_flags & 0b00000001) != 0;
    bool earth_oblate_spheroid = (resolution_component_flags & 0b00000010) != 0;
    bool u_v_components_relative_defined_grid = (resolution_component_flags & 0b00010000) != 0;
    bool reserved_3_4 = (resolution_component_flags & 0b00001100) != 0;
    bool reserved_6_8 = (resolution_component_flags & 0b11100000) != 0;

    outputBuffer << "Resolution and component flags:\n";
    outputBuffer << "Direction increments given: " << (direction_increments_given ? "Direction increments given" : "Direction increments not given") << "\n";
    outputBuffer << "Earth shape: " << (earth_oblate_spheroid ? "Earth assumed oblate spheroid with size as determined by IAU in 1965: 6378.160 km, 6356.775 km, f = 1/297.0" : "Earth assumed spherical with radius = 6367.47 km") << "\n";
    outputBuffer << "Reserved (3-4): " << (reserved_3_4 ? "Set to 1" : "Reserved (set to 0)") << "\n";
    outputBuffer << "u- and v-components of vector quantities: " << (u_v_components_relative_defined_grid ? "u- and v-components of vector quantities resolved relative to the defined grid in the direction of increasing x and y (or i and j) coordinates respectively" : "u- and v-components of vector quantities resolved relative to easterly and northerly directions") << "\n";
    outputBuffer << "Reserved (6-8): " << (reserved_6_8 ? "Set to 1" : "Reserved (set to 0)") << "\n";

    // Read latitude of last grid point
    int La2_millidegrees = readBytesAsInt(grib, 3);
    bool isSouthLatitude = (La2_millidegrees & 0x800000) != 0;
    La2_millidegrees &= 0x7FFFFF;
    double La2_degrees = static_cast<double>(La2_millidegrees) / 1000.0;
    outputBuffer << "Latitude of last grid point: " << La2_degrees << " degrees\n";

    // Read longitude of last grid point
    int Lo2_millidegrees = readBytesAsInt(grib, 3);
    bool isSouthLongitude = (Lo2_millidegrees & 0x800000) != 0;
    Lo2_millidegrees &= 0x7FFFFF;
    double Lo2_degrees = static_cast<double>(Lo2_millidegrees) / 1000.0;
    outputBuffer << "Longitude of last grid point: " << Lo2_degrees << " degrees\n";

    // Read longitudinal direction increment
    int DirectionIncrement = readBytesAsInt(grib, 2);
    double DirectionIncrementDegrees = static_cast<double>(DirectionIncrement) / 1000.0;
    outputBuffer << "Longitudinal Direction Increment: " << DirectionIncrementDegrees << " degrees\n";

    // Read latitudinal direction increment
    int LatitudinalDirection = readBytesAsInt(grib, 2);
    double LatitudinalDirectionDegrees = static_cast<double>(LatitudinalDirection) / 1000.0;
    outputBuffer << "Latitudinal Direction Increment: " << LatitudinalDirectionDegrees << " degrees\n";

    // Read scanning mode flags
    char byte_char;
    grib.read(&byte_char, 1);
    char scanning_mode_flags = byte_char;
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
    vector<int> row_points(num_rows);

    for (int i = 0; i < num_rows; ++i)
    {
        char bytes[2];
        grib.read(bytes, 2);
        int point_value = (static_cast<int>(static_cast<unsigned char>(bytes[0])) << 8) |
            (static_cast<int>(static_cast<unsigned char>(bytes[1])));

        if (point_value == 0) {
            --i;  // Powtórz odczyt dla tego samego wiersza
        }
        else {
            row_points[i] = point_value;
        }
    }

    for (int i = 0; i < num_rows; ++i)
    {
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

    // Mark the start of section 3
    grib.seekg(start_section_2 + section_2_length);
    uint32_t start_section_3 = grib.tellg();

    // Read the next three bytes into a 4-byte variable for section 3 length
    uint32_t section_3_length = readBytesAsInt(grib, 3);
    outputBuffer << "Section 3 length: " << section_3_length << "\n";

    // Read flag to decode
    int Flag_to_decode = readBytesAsInt(grib, 1);
    outputBuffer << "Flag to decode: " << Flag_to_decode << "\n";

    // Read binary scale factor
    int BinaryScaleFactor = readBytesAsInt(grib, 1);
    outputBuffer << "Binary Scale Factor: " << BinaryScaleFactor << "\n";

    outputFile << "==========================\n";
    outputFile << "== Section 3 Data ==\n";
    outputFile << "==========================\n\n";

    outputFile << outputBuffer.str();
    cout << outputBuffer.str();

    // Close files
    grib.close();
    outputFile.close();

    return 0;
}
