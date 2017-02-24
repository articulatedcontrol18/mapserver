

#include "map.h"

using namespace std;

void Map::printPoly(Polygon *poly)
{
    cout << "number of nodes: " << poly->numOfNodes << endl;
    cout << "Nodes:" << endl;
    for(int i = 0; i < poly->nodes.size(); i++){
        cout << "node: " << poly->nodes.at(i).id << " x: " << poly->nodes.at(i).x << " y: " << poly->nodes.at(i).y << endl;
	    }    
}

void Map::printMarking(Marking *marking)
{
    cout << "id: " << marking->id << endl;
    cout << "pos: (" << marking->x << "," << marking->y << ")" << endl; 
}

void Map::printMap()
{
    cout << "\nPolygons:" << endl;
    for(int i = 0; i < polygons.size();i++){
        printPoly(&polygons.at(i));
    }   
    cout << "\nMarkings:" << endl;
    for(int i = 0; i < markings.size();i++){
        printMarking(&markings.at(i));
    }
}

void Map::createPoly(ifstream &in, Polygon &poly)
{
    string str;
    int nodeCounter = 0;

    while(getline(in,str)){
        cout << str << endl;
        if(!str.compare(POLY_END)){
            poly.numOfNodes = nodeCounter;
            return;
        }else if(str.find(POLY_INSIDE) != string::npos){
            poly.allowedInside = true;
        }else if(str.find(POLY_OUTSIDE) != string::npos){
            poly.allowedInside = false;
		}else{            
            
            str.erase(remove(str.begin(), str.end(), ' '), str.end()); // remove all white spaces
            size_t index = str.find(',');

            int x = stoi(str.substr(0,index));
            int y = stoi(str.substr(index+1,str.length()-index));

			struct Node node;
            node.id = nodeCounter;
            node.x = x;
            node.y = y;
			poly.nodes.push_back(node);

            nodeCounter++;  
        }              
    }

}

void Map::createMarking(ifstream &in, Marking &marking)
{
    string str;
    getline(in,str);
    str.erase(remove(str.begin(), str.end(), ' '), str.end()); // remove all white spaces
    marking.id = stoi(str);
    getline(in,str);
    size_t index = str.find(',');
    marking.x = stoi(str.substr(0,index));
    marking.y = stoi(str.substr(index+1,str.length()-index));
    getline(in,str);
    if(str.compare(MARKING_END)){
        cout << "Something wrong with a marking.." << str << endl;
    }    
}

bool Map::isCommentLine(string &str)
{
    return str[0] == COMMENT_SIGN;
}

void Map::getMarkingPos(int id, int &x, int &y)
{
    for(int i = 0; i < markings.size(); i++){
        struct Marking marking = markings.at(i);
        if(marking.id == id){
            x = marking.x;
            y = marking.y;
            return;
        }
    }
    x = -1;
    y = -1;
}

bool Map::isPosInPoly(Polygon *poly, int x, int y)
{
    bool c = false;
    int i, j = 0;

    for (i = 0, j = poly->numOfNodes-1; i < poly->numOfNodes; j = i++) {
        
        struct Node prevNode, curNode;        
        prevNode = poly->nodes.at(j);
        curNode = poly->nodes.at(i);
        
        if(curNode.x == x && curNode.y == y){
            return true;
        }

        if(((curNode.y > y) != (prevNode.y > y)) &&
            (x < (prevNode.x - curNode.x) * (y - curNode.y) / (prevNode.y - curNode.y) + curNode.x)){
            c = !c;
        }
    }
    return c;
}


void Map::isForbiddenPos(int x, int y, bool &b)
{
    for(int i = 0; i < polygons.size(); i++){
        cout << isPosInPoly(&polygons.at(i), x, y) << " " << polygons.at(i).allowedInside << endl;
        if(isPosInPoly(&polygons.at(i), x, y) != polygons.at(i).allowedInside){
            b = true;
            return;
        }
    }
    b = false;
}

string Map::getexepath()
{
  char result[ PATH_MAX ];
  ssize_t count = readlink( "/proc/self/exe", result, PATH_MAX );
  return std::string( result, (count > 0) ? count : 0 );
}

Map::Map()
{	
    string executionPath = getexepath();

    cout << "execution path: " << executionPath << endl;

    string lookFor = "devel";
    int splitIndex = 0;
    for(int i = 0; i < executionPath.length(); i++){
        if(executionPath[i] == '/'){
            cout << "compare devel with : " << executionPath.substr(i+1,5) << endl;
            if(!executionPath.substr(i+1,5).compare(lookFor)){
                splitIndex = i;
                break;
            }
        }
    }

    cout << "split index: " << splitIndex << endl;

    string path = executionPath.substr(0,splitIndex) + "/src/mapserver/src/db.db";

    cout << path << endl;

    ifstream in(path);

    if(!in){
        cout << "Cannot open input file" << endl;
    }
	
	string str;
    while(getline(in,str)){
       
        if(isCommentLine(str)){
            continue;
        }            
        
		if(!str.compare(POLY_START)){
            struct Polygon poly;
            createPoly(in, poly);
            polygons.push_back(poly);
        }else if(!str.compare(MARKING_START)){
            struct Marking marking;
            createMarking(in, marking);
            markings.push_back(marking);
		}else if(!str.empty()){
            cout << "Can't parse line: " << str << endl;
        }       
    }
    cout << "Created map" << endl;
}

/*
int main()
{
    Map map;
    map.printMap();
    int x,y;
    map.getMarkingPos(1,x,y);
    cout << "position of marking id 1; " << x << "," << y << endl;
    bool inside = map.isPosInPoly(&map.polygons.at(0), 2, 2);
    cout << "pos(2,2) inside poly1? " << inside << endl; 
    inside = map.isPosInPoly(&map.polygons.at(0), 0, 0);
    cout << "pos(0,0) inside poly1? " << inside << endl; 
    inside = map.isPosInPoly(&map.polygons.at(0), 3, 0);
    cout << "pos(3,0) inside poly1? " << inside << endl; 
    inside = map.isPosInPoly(&map.polygons.at(0), 3, 3);
    cout << "pos(3,3) inside poly1? " << inside << endl; 
    inside = map.isPosInPoly(&map.polygons.at(0), 0, 3);
    cout << "pos(0,3) inside poly1? " << inside << endl;

    inside = map.isPosInPoly(&map.polygons.at(1), 4, 0);
    cout << "pos(4,0) no? " << inside << endl; 


    inside = map.isPosInPoly(&map.polygons.at(1), 1, 2);
    cout << "pos(1,2) no " << inside << endl; 


    inside = map.isPosInPoly(&map.polygons.at(1), 5, 2);
    cout << "pos(5,2) no" << inside << endl; 


    inside = map.isPosInPoly(&map.polygons.at(1), 4, 0);
    cout << "pos(4,0) no " << inside << endl;


    inside = map.isPosInPoly(&map.polygons.at(1), 1, 0);
    cout << "pos(1,0) yes " << inside << endl;  


    inside = map.isPosInPoly(&map.polygons.at(1), 2, 0);
    cout << "pos(2,0) yes" << inside << endl; 


    inside = map.isPosInPoly(&map.polygons.at(1), 3, 0);
    cout << "pos(3,0) yes " << inside << endl; 


    inside = map.isPosInPoly(&map.polygons.at(1), 3, 2);
    cout << "pos(3,2) on edge no crossing edge " << inside << endl; 

    inside = map.isPosInPoly(&map.polygons.at(1), 4, 1);
    cout << "pos(4,1) on edge no crossing edge.. " << inside << endl; 

}





*/
