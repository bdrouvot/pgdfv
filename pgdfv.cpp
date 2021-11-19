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
        cout << "Bertrand Drouvot: https://bdrouvot.github.io" << endl;
}

void help()
{
        cout << "-df     Path to a datafile (mandatory if indexfile is used)" << endl;
        cout << "-if     Path to an indexfile" << endl;
        cout << "-b      Block size (default 8192)" << endl;
        cout << "-bg     What background color do you use? WHITE (default) or BLACK?" << endl;
        cout << "-ls     Set line size (default 128)" << endl;
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

int *getItemIdData(int p_blocknumber, int p_blocksize, int p_rownumber, ifstream& p_datafile)
{
// go to ItemIdData
// and returns lp_len, lp_off and lp_flags

static int result[3];
p_datafile.seekg((p_blocknumber * p_blocksize) + 24 + (4 * p_rownumber)); //set pointer at start

char row_itemdata[4];
int row_itemdataI;
p_datafile.read(row_itemdata, 4); // read 4 bytes
memcpy(&row_itemdataI, row_itemdata, sizeof(row_itemdata));

unsigned first15bits = getbits(row_itemdataI, 0, 15);
unsigned last15bits = getbits(row_itemdataI, 17, 15);
unsigned bits16and17 = getbits(row_itemdataI, 15, 2);

int lp_lenI = (int)last15bits;    //lp_len
int lp_offI = (int)first15bits;   //lp_off
int lp_flagsI = (int)bits16and17; //lp_flags

result[0]=lp_lenI;
result[1]=lp_offI;
result[2]=lp_flagsI;

return result;
}

int *getOpaqueData(int p_blocknumber, int p_blocksize, int p_offset, ifstream& p_datafile)
{
// go to getOpaqueData
// and returns btpo_prev, btpo_next, tree_level and btpo_flags
static int result[4];

p_datafile.seekg((p_blocknumber * p_blocksize) + p_offset); //set pointer at start of OpaqueData

char btpo_prev[4];
int btpo_prevI;
p_datafile.read(btpo_prev, 4); // read 4 bytes
memcpy(&btpo_prevI, btpo_prev, sizeof(btpo_prev));

p_datafile.seekg((p_blocknumber * p_blocksize) + p_offset + 4); 

char btpo_next[4];
int btpo_nextI;
p_datafile.read(btpo_next, 4); // read 4 bytes
memcpy(&btpo_nextI, btpo_next, sizeof(btpo_next));

p_datafile.seekg((p_blocknumber * p_blocksize) + p_offset + 8);

char tree_level[4];
int tree_levelI;
p_datafile.read(tree_level, 4); // read 4 bytes
memcpy(&tree_levelI, tree_level, sizeof(tree_level));

p_datafile.seekg((p_blocknumber * p_blocksize) + p_offset + 12);

char btpo_flags[2];
int btpo_flagsI;
p_datafile.read(btpo_flags, 2);
memcpy(&btpo_flagsI, btpo_flags, sizeof(btpo_flags));

result[0]=btpo_prevI;
result[1]=btpo_nextI;
result[2]=tree_levelI;
result[3]=btpo_flagsI;

return result;
}

int *getHeapTupleHeaderData(int p_blocknumber, int p_blocksize, int p_lp_offset, ifstream& p_datafile)
{
// go to HeapTupleHeaderData
// and returns t_xmin and t_xmax
static int result[2];

p_datafile.seekg((p_blocknumber * p_blocksize) + p_lp_offset); //set pointer at start of t_xmin

char t_xmin[4];
int t_xminI;
p_datafile.read(t_xmin, 4); // read 4 bytes
memcpy(&t_xminI, t_xmin, sizeof(t_xmin));

p_datafile.seekg((p_blocknumber * p_blocksize) + p_lp_offset + 4); //set pointer at start of t_xmax

char t_xmax[4];
int t_xmaxI;
p_datafile.read(t_xmax, 4); // read 4 bytes
memcpy(&t_xmaxI, t_xmax, sizeof(t_xmax));

result[0]=t_xminI;
result[1]=t_xmaxI;

return result;
}

int *getPageHeaderData(int p_blocknumber, int p_blocksize, ifstream& p_datafile)
{
// go to PageHeaderData
// and returns pd_lower, pd_upper and pd_special
static int result[2];

p_datafile.seekg((p_blocknumber * p_blocksize) + 12); //set pointer at start of pd_lower
char pd_lower[2];
int pd_lowerI;
p_datafile.read(pd_lower, 2); // read 2 bytes
memcpy(&pd_lowerI, pd_lower, sizeof(pd_lower));

p_datafile.seekg((p_blocknumber * p_blocksize) + 14); //set pointer at start of pd_upper
char pd_upper[2];
int pd_upperI;
p_datafile.read(pd_upper, 2); // read 2 bytes
memcpy(&pd_upperI, pd_upper, sizeof(pd_upper));

p_datafile.seekg((p_blocknumber * p_blocksize) + 16); //set pointer at start of pd_special
char pd_special[2];
int pd_specialI;
p_datafile.read(pd_special, 2); // read 2 bytes
memcpy(&pd_specialI, pd_special, sizeof(pd_special));

result[0]=pd_lowerI;
result[1]=pd_upperI;
result[2]=pd_specialI;

return result;
}

void display_block(map<int, map<int, int>> &p_pct_reuse_and_gt_50, map<int, map<int, int>> &p_pct_reuse_and_lt_50,map<int, string> &p_colorThresholds, int p_max_9_to_display, int p_pct_row_current, int p_pct_freeI) {

if (p_pct_row_current < 50)
 {
  if (p_pct_freeI >= 75)
   {
     cout << getColor(p_colorThresholds[75], false, true) << p_max_9_to_display << "\033[0m";
     p_pct_reuse_and_lt_50[75][p_max_9_to_display]++;
   }
   else if (p_pct_freeI >= 50)
   {
     cout << getColor(p_colorThresholds[50], false, true) << p_max_9_to_display << "\033[0m";
     p_pct_reuse_and_lt_50[50][p_max_9_to_display]++;
   }
   else if (p_pct_freeI >= 25)
   {
     cout << getColor(p_colorThresholds[25], false, true) << p_max_9_to_display << "\033[0m";
     p_pct_reuse_and_lt_50[25][p_max_9_to_display]++;
   }
   else
   {
     cout << getColor(p_colorThresholds[0], false, true) << p_max_9_to_display << "\033[0m";
     p_pct_reuse_and_lt_50[0][p_max_9_to_display]++;
   }
 }
 else
 {
  if (p_pct_freeI >= 75)
   {
    cout << getColor(p_colorThresholds[75], false, false) << p_max_9_to_display << "\033[0m";
    p_pct_reuse_and_gt_50[75][p_max_9_to_display]++;
   }
   else if (p_pct_freeI >= 50)
    {
     cout << getColor(p_colorThresholds[50], false, false) << p_max_9_to_display << "\033[0m";
     p_pct_reuse_and_gt_50[50][p_max_9_to_display]++;
    }
  else if (p_pct_freeI >= 25)
   {
     cout << getColor(p_colorThresholds[25], false, false) << p_max_9_to_display << "\033[0m";
     p_pct_reuse_and_gt_50[25][p_max_9_to_display]++;
   }
   else
  {
   cout << getColor(p_colorThresholds[0], false, false) << p_max_9_to_display << "\033[0m";
   p_pct_reuse_and_gt_50[0][p_max_9_to_display]++;
  }
}
}

void display_summary(string p_file_type,map<int, map<int, int>> &p_pct_reuse_and_gt_50, map<int, map<int, int>> &p_pct_reuse_and_lt_50,map<int, string> &p_colorThresholds) {

map<int, string> colorThresholds = p_colorThresholds;
map<int, map<int, int>>::iterator iter;

if (p_file_type == "index") {
cout << "M stands for meta-page" << endl;
cout << "R stands for root page" << endl;
cout << "? stands for non leaf, non root, non meta" << endl;
}

// Blocks with more than 50% rows current
for (int j = 0; j < 100; j += 25)
 {
   iter = p_pct_reuse_and_gt_50.find(j);
   if (iter != p_pct_reuse_and_gt_50.end())
   {
      map<int, int>::iterator innerIter;
      for (int k = 0; k < 10; k += 1)
      {
        innerIter = iter->second.find(k);
        if (innerIter != iter->second.end())
        {
          if (p_file_type == "data") {
	       cout << getColor(p_colorThresholds[j], false, false) << innerIter->first
               << "\033[0m"
               << " means blocks with free space in [" << j << "%," << j + 25 << "%], more than 50% rows current and unused in [" << k * 10 << "%," << (k + 1) * 10 << "%]: Total => " << innerIter->second << endl; } else {
	       cout << getColor(p_colorThresholds[j], false, false) << innerIter->first
               << "\033[0m"
               << " means blocks with free space in [" << j << "%," << j + 25 << "%], more than 50% rows current and HOT redirect in [" << k * 10 << "%," << (k + 1) * 10 << "%]: Total => " << innerIter->second << endl; }
        }
      }
     }
   }

 // Blocks with less than 50% rows current
for (int j = 0; j < 100; j += 25)
 {
   iter = p_pct_reuse_and_lt_50.find(j);
   if (iter != p_pct_reuse_and_lt_50.end())
   {
     map<int, int>::iterator innerIter;
     for (int k = 0; k < 10; k += 1)
     {
       innerIter = iter->second.find(k);
       if (innerIter != iter->second.end())
       {
	if (p_file_type == "data") {
         cout << getColor(p_colorThresholds[j], false, true) << innerIter->first
              << "\033[0m"
              << " means blocks with free space in [" << j << "%," << j + 25 << "%], less than 50% rows current and unused in [" << k * 10 << "%," << (k + 1) * 10 << "%]: Total => " << innerIter->second << endl; } else {
              cout << getColor(p_colorThresholds[j], false, true) << innerIter->first
              << "\033[0m"
              << " means blocks with free space in [" << j << "%," << j + 25 << "%], less than 50% rows current and HOT redirect in [" << k * 10 << "%," << (k + 1) * 10 << "%]: Total => " << innerIter->second << endl; }
       }
     }
   }
}
}

void readData(string p_fname, int p_blockSize, int lineS,map<int, string> &p_colorThresholds)
{
        int blockSize = p_blockSize; //block size

        string fileNameDbf = p_fname;
        int lineSize = lineS;

        map<int, map<int, int>> pct_reuse_and_gt_50;
        map<int, map<int, int>> pct_reuse_and_lt_50;

        map<int, string> colorThresholds = p_colorThresholds;

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
                int rows_unusable = 0;

                // read pd_lower and pd_upper from PageHeaderData
                int *PageHeaderData;
		        PageHeaderData = getPageHeaderData(i,blockSize,datafile);
                int pd_lowerI = *(PageHeaderData+0);
                int pd_upperI = *(PageHeaderData+1);

                int numOfRowsI = (pd_lowerI - 24) / 4; // Number of rows

                int pct_freeI;
                pct_freeI = (pd_upperI - pd_lowerI) * 100 / blockSize; // Free percent

                for (int row = 0; row < numOfRowsI; row++)
                {
                    // go to ItemIdData to get the offset of the row
                    int *ItemIdData;
                    ItemIdData = getItemIdData(i,blockSize,row,datafile);
                    int lp_lenI = *(ItemIdData+0);
                    int lp_offI = *(ItemIdData+1);
                    int lp_flagsI = *(ItemIdData+2);

                    if (lp_flagsI == 1)
                    {
                        // let's read HeapTupleHeaderData
                        int *HeapTupleHeaderData;
                        HeapTupleHeaderData = getHeapTupleHeaderData(i,blockSize,lp_offI,datafile);

                        int t_xminI = *(HeapTupleHeaderData+0);
                        int t_xmaxI = *(HeapTupleHeaderData+1);

                        if (t_xmaxI == 0)
                        {
                            rows_current++;
                        }
                    }
                    else if (lp_flagsI == 0)
                    {
                        rows_unusable++;
                    }
                }

                int pct_row_current = rows_current * 100 / numOfRowsI;
                int pct_row_unusable = rows_unusable * 100 / numOfRowsI;
                int pct_row_unusable_display = pct_row_unusable / 10;

                int max_9_to_display = (9 < pct_row_unusable_display) ? 9 : pct_row_unusable_display;

                if (i > 0 && i % lineSize == 0)
                {
                    cout << endl;
                    printf("%08d - %08d:  ", i + 1, i + lineSize);
                }

		        display_block(pct_reuse_and_gt_50,pct_reuse_and_lt_50,colorThresholds,max_9_to_display,pct_row_current,pct_freeI);

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

	display_summary("data",pct_reuse_and_gt_50,pct_reuse_and_lt_50,colorThresholds);
}


void readIndex(string p_ifname,string p_dfname, int p_blockSize, int lineS,map<int, string> &p_colorThresholds)
{
        int blockSize = p_blockSize; //block size

        string datafileNameDbf = p_dfname;
        string indexfileNameDbf = p_ifname;
        int lineSize = lineS;

        map<int, string> colorThresholds = p_colorThresholds;
        map<int, map<int, int>> idx_pct_reuse_and_gt_50;
        map<int, map<int, int>> idx_pct_reuse_and_lt_50;

        ifstream indexfile(indexfileNameDbf.c_str(), ios::binary);
        ifstream datafile(datafileNameDbf.c_str(), ios::binary);
        if (indexfile.is_open())
        {
            int i = 0; 
            while (!indexfile.eof() && indexfile.good())
            {
                if (i == 0)
                {
                    printf("%08d - %08d:  ", i + 1, i + lineSize);
                }

                // read pd_lower and pd_upper from PageHeaderData
                int *PageHeaderData;
                PageHeaderData = getPageHeaderData(i,blockSize,indexfile);
                int pd_lowerI = *(PageHeaderData+0);
                int pd_upperI = *(PageHeaderData+1);
                int pd_specialI = *(PageHeaderData+2);
		        //cout << pd_specialI << endl;
		        // check type of index block

	            int *OpaqueData;
		        OpaqueData = getOpaqueData(i,blockSize,pd_specialI,indexfile);
                int btpo_prevI = *(OpaqueData+0);
                int btpo_nextI = *(OpaqueData+1);
                int tree_levelI = *(OpaqueData+2);
                int btpo_flagsI = *(OpaqueData+3);

                //cout << "Prev: " << btpo_prevI << endl;
                //cout << "Next: " << btpo_nextI << endl;
                //cout << "Tree Level: " << tree_levelI << endl;
                //cout << "btpo_flags: " << btpo_flagsI << endl;


                // check btpo_flags with that in mind

                /* Bits defined in btpo_flags */
                //#define BTP_LEAF                (1 << 0)        /* leaf page, i.e. not internal page */
                //#define BTP_ROOT                (1 << 1)        /* root page (has no parent) */
                //#define BTP_DELETED             (1 << 2)        /* page has been deleted from tree */
                //#define BTP_META                (1 << 3)        /* meta-page */
                //#define BTP_HALF_DEAD   (1 << 4)        /* empty, but still in tree */
                //#define BTP_SPLIT_END   (1 << 5)        /* rightmost page of split group */
                //#define BTP_HAS_GARBAGE (1 << 6)        /* page has LP_DEAD tuples */
                //#define BTP_INCOMPLETE_SPLIT (1 << 7)   /* right sibling's downlink is missing */

                if (tree_levelI != 0 || btpo_flagsI==8) {
                    if (i > 1 && i % lineSize == 0)
                    {
                      cout << endl;
                      printf("%08d - %08d:  ", i + 1, i + lineSize);
                     }
	                if (btpo_flagsI==8) { cout << "M"; }
                    else if (btpo_flagsI==2) { cout << "R"; }
                    else { cout << "?"; }
                } else if (btpo_flagsI==1){

                    // it is a leaf page
                    int idx_rows_current = 0;
                    int idx_rows_HOT_redirect = 0;

                    int numOfIndexI = (pd_lowerI - 24) / 4; // Number of Index entries

                    int idx_pct_freeI;
                    idx_pct_freeI = (pd_upperI - pd_lowerI) * 100 / blockSize; // Free percent

                    for (int idx = 0; idx < numOfIndexI; idx++)
                    {
                        // go to ItemIdData to get the offset of the index entry
                        int *ItemIdData;
                        ItemIdData = getItemIdData(i,blockSize,idx,indexfile);
                        int lp_lenI = *(ItemIdData+0);
                        int lp_offI = *(ItemIdData+1);
                        int lp_flagsI = *(ItemIdData+2);

                        indexfile.seekg((i * blockSize) + lp_offI + 2); //set pointer at start of t_ctid
                    
                        char t_cid[6];
                        int t_cidI;
                        indexfile.read(t_cid, 6); // read 6 bytes
                        memcpy(&t_cidI, t_cid, sizeof(t_cid));

                        unsigned last16bits = getbits(t_cidI, 32, 16);
                        int last16bitsI = (int)last16bits;

                        unsigned first16bits = getbits(t_cidI, 0, 16);
                        int first16bitsI = (int)first16bits; // block number

                        unsigned middle16bits = getbits(t_cidI, 16, 16);
                        int middle16bitsI = (int)middle16bits; // record number

                        //cout << "(" << first16bitsI << "," << middle16bits << ")" << endl;

                        // go to ItemIdData to get the offset of the row
                        int *RowItemIdData;
                        RowItemIdData = getItemIdData(first16bitsI,blockSize,middle16bits-1,datafile);
                        int rowlp_lenI = *(RowItemIdData+0);
                        int rowlp_offI = *(RowItemIdData+1);
                        int rowlp_flagsI = *(RowItemIdData+2);

                        if (rowlp_flagsI == 1)
                        {
                            // let's read HeapTupleHeaderData
                            int *RowHeapTupleHeaderData;
                            RowHeapTupleHeaderData = getHeapTupleHeaderData(first16bitsI,blockSize,rowlp_offI,datafile);

                            int rowt_xminI = *(RowHeapTupleHeaderData+0);
                            int rowt_xmaxI = *(RowHeapTupleHeaderData+1);

                            if (rowt_xmaxI == 0)
                            {
                                idx_rows_current++;
                            }
                        }
                        else if (rowlp_flagsI == 2 && rowlp_lenI == 0)
                        {
                            idx_rows_HOT_redirect++;
                        }
                    }

                    int idx_pct_row_current = idx_rows_current * 100 / numOfIndexI;
                    int idx_pct_rows_HOT_redirect = idx_rows_HOT_redirect * 100 / numOfIndexI;
                    int idx_pct_row_HOT_redirect_display = idx_pct_rows_HOT_redirect / 10;
                    int idx_max_9_to_display = (9 < idx_pct_row_HOT_redirect_display) ? 9 : idx_pct_row_HOT_redirect_display;

                    if (i > 1 && i % lineSize == 0)
                    {
                        cout << endl;
                        printf("%08d - %08d:  ", i + 1, i + lineSize);
                    }

                    display_block(idx_pct_reuse_and_gt_50,idx_pct_reuse_and_lt_50,colorThresholds,idx_max_9_to_display,idx_pct_row_current,idx_pct_freeI);

                } // end if leaf page

                i++;
                indexfile.seekg(i * blockSize);
                char istheEnd[1];
                indexfile.read(istheEnd, 1);
            }
        }
        indexfile.close();

        cout << endl
             << endl
             << "Legend and Summary: "
             << endl
             << endl;

        display_summary("index",idx_pct_reuse_and_gt_50,idx_pct_reuse_and_lt_50,colorThresholds);
}

int main(int argc, char *argv[])
{
    string dataFile = "-df";  //switch for datafile path
    string indexFile = "-if";  //switch for index file path
    string block = "-b";     //switch for block size
    string bgColor = "-bg";  //what backgroupnd color is used
    string lineSize = "-ls"; //size of the line to display

    int lineS = 128;
    int blockSize = 8192; //block size
    string datafileName;
    string indexfileName;

    map<int, string> colorThresholds;

    colorThresholds[75] = "GREEN";
    colorThresholds[50] = "CYAN";
    colorThresholds[25] = "YELLOW";
    colorThresholds[0] = "RED";

    if (argc >= 2)
    {
        for (int i = 1; i < argc; i++)
        {
            if (argv[i] == dataFile)
            {
                datafileName = argv[i + 1];
            }
            else if (argv[i] == indexFile)
            {
                indexfileName = argv[i + 1];
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
	if (!indexfileName.empty()) {
	    if (datafileName.empty()) {
            cout << "*** Mandatory data file empty ***" << endl;
	        help();
            return 1;
	    }
	    cout << endl;
        cout << "********** For Data: " << datafileName << " **********" << endl;
        cout << endl;
        readData(datafileName, blockSize, lineS,colorThresholds);

        cout << endl;
        cout << "********** For Index: " << indexfileName << " **********" << endl;
        cout << endl;
        readIndex(indexfileName,datafileName, blockSize, lineS,colorThresholds);
	} else {
          cout << endl;
          cout << "********** For Data: " << datafileName << " **********" << endl;
          cout << endl;
          readData(datafileName, blockSize, lineS,colorThresholds);
	}
	banner();
    return 0;
}
