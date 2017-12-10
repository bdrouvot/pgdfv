#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>
#include <sstream>
#include <cmath>
#include <map>
#include <limits.h>

using namespace std;

string bgColor = "WHITE";

void banner()
{
        cout << endl
             << endl
             << "FOR STUDY PURPOSE ONLY" << endl;
        cout << "Bertrand Drouvot: https://bdrouvot.wordpress.com/" << endl;
}

void help()
{
        cout << "-f     Path to a file" << endl;
        cout << "-b     Block size" << endl;
        cout << "-bg    What background color do you use? WHITE (default) or BLACK?" << endl;
        cout << "-ls    Set line size (default 128)" << endl;
        banner();
}

string getColor(string name, bool empty, bool manyNotCurrent)
{
        map<string, string> colors;
        colors["BLACK"] = "\033[30";
        colors["RED"] = "\033[31";
        colors["GREEN"] = "\033[32";
        colors["YELLOW"] = "\033[33";
        colors["BLUE"] = "\033[34";
        colors["MAGENTA"] = "\033[35";
        colors["CYAN"] = "\033[36";
        colors["WHITE"] = "\033[37";

        if (colors.count(name) > 0 && !empty && !manyNotCurrent)
        {
                return colors[name] + "m";
        }
        else if (colors.count(name) > 0 && empty)
        {
                if (bgColor == "BLACK")
                        return colors[name] + ";5;47m";
                else
                        return colors[name] + ";5;40m";
        }
        else if (colors.count(name) > 0 && !empty && manyNotCurrent)
        {
                if (bgColor == "BLACK")
                        return colors[name] + ";7;47m";
                else
                        return colors[name] + ";7;40m";
        }
        else
        {
                return "\033[0m";
        }
}

unsigned getbits(unsigned value, unsigned offset, unsigned n)
{
        const unsigned max_n = CHAR_BIT * sizeof(unsigned);
        if (offset >= max_n)
                return 0; /* value is padded with infinite zeros on the left */
        value >>= offset; /* drop offset bits */
        if (n >= max_n)
                return value;                /* all  bits requested */
        const unsigned mask = (1u << n) - 1; /* n '1's */
        return value & mask;
}

void readData(string p_fname, int p_blockSize, int lineS)
{
        int pd_lowerOffset = 12;     //offset for pd_lower
        int pd_upperOffset = 14;     //offset for pd_upper
        int blockSize = p_blockSize; //block size

        string fileNameDbf = p_fname;
        int lineSize = lineS;

        int free_gt_75_and_gt_50_pct_rows_current = 0;
        int free_gt_50_and_gt_50_pct_rows_current = 0;
        int free_gt_25_and_gt_50_pct_rows_current = 0;
        int free_gt_00_and_gt_50_pct_rows_current = 0;

        int free_gt_75_and_lt_50_pct_rows_current = 0;
        int free_gt_50_and_lt_50_pct_rows_current = 0;
        int free_gt_25_and_lt_50_pct_rows_current = 0;
        int free_gt_00_and_lt_50_pct_rows_current = 0;

        map<int, map<int, int>> pct_reuse_and_gt_50;
        map<int, map<int, int>> pct_reuse_and_lt_50;
        map<int, string> colorThresholds;

        colorThresholds[75] = "GREEN";
        colorThresholds[50] = "CYAN";
        colorThresholds[25] = "YELLOW";
        colorThresholds[0] = "RED";

        ifstream datafile(fileNameDbf.c_str(), ios::binary);
        if (datafile.is_open())
        {
                int i = 0;
                while (!datafile.eof() && datafile.good())
                {

                        if (i == 0)
                        {
                                printf("%08d - %08d:  ", i + 1, i + lineSize);
                        }

                        int rows_current = 0;
                        int rows_reusable = 0;

                        char pd_lower[2];
                        int pd_lowerI;
                        datafile.seekg(i * blockSize + pd_lowerOffset); //set pointer at offset 12
                        datafile.read(pd_lower, 2);                     // read 2 bytes
                        memcpy(&pd_lowerI, pd_lower, sizeof(pd_lower));

                        char pd_upper[2];
                        int pd_upperI;
                        int numOfRowsI;
                        datafile.seekg(i * blockSize + pd_upperOffset); //set pointer at offset 14
                        datafile.read(pd_upper, 2);                     // read 2 bytes
                        memcpy(&pd_upperI, pd_upper, sizeof(pd_upper));

                        numOfRowsI = (pd_lowerI - 24) / 4; // Number of rows

                        int pct_freeI;
                        pct_freeI = (pd_upperI - pd_lowerI) * 100 / blockSize; // Free percent

                        for (int row = 0; row < numOfRowsI; row++)
                        {
                                // go to ItemIdData
                                datafile.seekg((i * blockSize) + 24 + (4 * row)); //set pointer at start
                                char row_itemdata[4];
                                int row_itemdataI;
                                datafile.read(row_itemdata, 4); // read 4 bytes
                                memcpy(&row_itemdataI, row_itemdata, sizeof(row_itemdata));

                                unsigned first15bits = getbits(row_itemdataI, 0, 15);
                                unsigned last15bits = getbits(row_itemdataI, 17, 15);
                                unsigned bits16and17 = getbits(row_itemdataI, 15, 2);

                                int lp_lenI = (int)last15bits;    //lp_len
                                int lp_offI = (int)first15bits;   //lp_off
                                int lp_flagsI = (int)bits16and17; //lp_flags

                                if (lp_flagsI == 1)
                                {
                                        // let's read HeapTupleHeaderData

                                        datafile.seekg((i * blockSize) + lp_offI); //set pointer at start of t_xmin
                                        char t_xmin[4];
                                        int t_xminI;
                                        datafile.read(t_xmin, 4); // read 4 bytes
                                        memcpy(&t_xminI, t_xmin, sizeof(t_xmin));

                                        datafile.seekg((i * blockSize) + lp_offI + 4); //set pointer at start of t_xmax
                                        char t_xmax[4];
                                        int t_xmaxI;
                                        datafile.read(t_xmax, 4); // read 4 bytes
                                        memcpy(&t_xmaxI, t_xmax, sizeof(t_xmax));

                                        if (t_xmaxI == 0)
                                        {
                                                rows_current++;
                                        }
                                }
                                else if (lp_flagsI == 0)
                                {
                                        rows_reusable++;
                                }
                        }

                        int pct_row_current = rows_current * 100 / numOfRowsI;
                        int pct_row_reusable = rows_reusable * 100 / numOfRowsI;
                        int pct_row_reusable_display = pct_row_reusable / 10;

                        int max_9_to_display = (9 < pct_row_reusable_display) ? 9 : pct_row_reusable_display;

                        if (i > 0 && i % lineSize == 0)
                        {
                                cout << endl;
                                printf("%08d - %08d:  ", i + 1, i + lineSize);
                        }

                        if (pct_row_current < 50)
                        {
                                if (pct_freeI >= 75)
                                {
                                        cout << getColor(colorThresholds[75], false, true) << max_9_to_display << "\033[0m";
                                        free_gt_75_and_lt_50_pct_rows_current++;
                                        pct_reuse_and_lt_50[75][max_9_to_display] = free_gt_75_and_lt_50_pct_rows_current;
                                }
                                else if (pct_freeI >= 50)
                                {
                                        cout << getColor(colorThresholds[50], false, true) << max_9_to_display << "\033[0m";
                                        free_gt_50_and_lt_50_pct_rows_current++;
                                        pct_reuse_and_lt_50[50][max_9_to_display] = free_gt_50_and_lt_50_pct_rows_current;
                                }
                                else if (pct_freeI >= 25)
                                {
                                        cout << getColor(colorThresholds[25], false, true) << max_9_to_display << "\033[0m";
                                        free_gt_25_and_lt_50_pct_rows_current++;
                                        pct_reuse_and_lt_50[25][max_9_to_display] = free_gt_25_and_lt_50_pct_rows_current;
                                }
                                else
                                {
                                        cout << getColor(colorThresholds[0], false, true) << max_9_to_display << "\033[0m";
                                        free_gt_00_and_lt_50_pct_rows_current++;
                                        pct_reuse_and_lt_50[0][max_9_to_display] = free_gt_00_and_lt_50_pct_rows_current;
                                }
                        }
                        else
                        {
                                if (pct_freeI >= 75)
                                {
                                        cout << getColor(colorThresholds[75], false, false) << max_9_to_display << "\033[0m";
                                        free_gt_75_and_gt_50_pct_rows_current++;
                                        pct_reuse_and_gt_50[75][max_9_to_display] = free_gt_75_and_gt_50_pct_rows_current;
                                }
                                else if (pct_freeI >= 50)
                                {
                                        cout << getColor(colorThresholds[50], false, false) << max_9_to_display << "\033[0m";
                                        free_gt_50_and_gt_50_pct_rows_current++;
                                        pct_reuse_and_gt_50[50][max_9_to_display] = free_gt_50_and_gt_50_pct_rows_current;
                                }
                                else if (pct_freeI >= 25)
                                {
                                        cout << getColor(colorThresholds[25], false, false) << max_9_to_display << "\033[0m";
                                        free_gt_25_and_gt_50_pct_rows_current++;
                                        pct_reuse_and_gt_50[25][max_9_to_display] = free_gt_25_and_gt_50_pct_rows_current;
                                }
                                else
                                {
                                        cout << getColor(colorThresholds[0], false, false) << max_9_to_display << "\033[0m";
                                        free_gt_00_and_gt_50_pct_rows_current++;
                                        pct_reuse_and_gt_50[0][max_9_to_display] = free_gt_00_and_gt_50_pct_rows_current;
                                }
                        }

                        i++;
                        datafile.seekg(i * blockSize);
                        char istheEnd[1];
                        datafile.read(istheEnd, 1);
                }
        }

        datafile.close();

        cout << endl
             << endl
             << "Legend and Summary: "
             << endl
             << endl;

        map<int, map<int, int>>::iterator iter;

        // Blocks with more than 50% rows current
        for (int j = 0; j < 100; j += 25)
        {
                iter = pct_reuse_and_gt_50.find(j);
                if (iter != pct_reuse_and_gt_50.end())
                {
                        map<int, int>::iterator innerIter;
                        for (int k = 0; k < 10; k += 1)
                        {
                                innerIter = iter->second.find(k);
                                if (innerIter != iter->second.end())
                                {
                                        cout << getColor(colorThresholds[j], false, false) << innerIter->first
                                             << "\033[0m"
                                             << " means blocks with free space in [" << j << "%," << j + 25 << "%], more than 50% rows current and unused in [" << k * 10 << "%," << (k + 1) * 10 << "%]: Total => " << innerIter->second << endl;
                                }
                        }
                }
        }

        // Blocks with less than 50% rows current
        for (int j = 0; j < 100; j += 25)
        {
                iter = pct_reuse_and_lt_50.find(j);
                if (iter != pct_reuse_and_lt_50.end())
                {
                        map<int, int>::iterator innerIter;
                        for (int k = 0; k < 10; k += 1)
                        {
                                innerIter = iter->second.find(k);
                                if (innerIter != iter->second.end())
                                {
                                        cout << getColor(colorThresholds[j], false, true) << innerIter->first
                                             << "\033[0m"
                                             << " means blocks with free space in [" << j << "%," << j + 25 << "%], less than 50% rows current and unused in [" << k * 10 << "%," << (k + 1) * 10 << "%]: Total => " << innerIter->second << endl;
                                }
                        }
                }
        }

        banner();
}

int main(int argc, char *argv[])
{
        string dataFile = "-f";  //switch for datafile path
        string block = "-b";     //switch for block size
        string bgColor = "-bg";  //what backgroupnd color is used
        string lineSize = "-ls"; //size of the line to display

        int lineS = 128;

        int blockSize; //block size
        string fileName;

        if (argc >= 4)
        {
                for (int i = 1; i < argc; i++)
                {
                        if (argv[i] == dataFile)
                        {
                                fileName = argv[i + 1];
                        }
                        else if (argv[i] == block)
                        {
                                blockSize = stoi(argv[i + 1]);
                        }
                        else if (argv[i] == lineSize)
                        {
                                lineS = stoi(argv[i + 1]);
                        }
                }
        }
        else
        {
                help();
                return 1;
        }
        readData(fileName, blockSize, lineS);
        return 0;
}
