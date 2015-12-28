#include <fstream>
#include <iostream>
#include <string>

using namespace std;

class SokobanObject
{
	public:
		SokobanObject();
}

class Tile
{
	public:
		int x,y;
		SokobanObject object;
		bool hasObject, isGoal, isSolid;
		
		Tile(int x, int y, bool isGoal, bool isSolid)
		{
			this->x = x;
			this->y = y;
			this->isGoal = isGoal;
			this->isSolid = isSolid;
		}
		
}

class Map
{
	public:
		string mapName;
		vector<vector<Tile>> grid;
	
		Map(string mapName)
		{
			string line;
			int row = 0;
			ifstream mapFile(mapName + "_info.txt");
			if(mapFile.is_open())
			{
				while(getline(mapFile,line)
				{
					vector<Tile> mapRow;
					for(int i=0;i<line.length(); i++)
					{
						if(line[i] == "-")
						{
							mapRow.push_back(Tile(i,row,false, false));
						}
						else if(line[i] == "O")
						{
							mapRow.push_back(Tile(i,row,true, false));
						}
						else if(line[i] == "R")
						{
							//ADD ROBOT TO TILE
							mapRow.push_back(Tile(i,row,false, false));
						}
						else if(line[i] == "C")
						{
							//ADD BOX TO TILE
							mapRow.push_back(Tile(i,row,false, false));
						}
						else if(line[i] == "P")
						{
							mapRow.push_back(Tile(i,row,false, true));
						}
					}
					grid.push_back(mapRow);
					row++;
				}
				mapFile.close();
			}
			else {
				cout << "Unable to find map file." << endl;
			}
		}
}
