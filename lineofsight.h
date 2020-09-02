/* (c) 2017. Andreychuk A.
 * This class implements line-of-sight function for a variable size of agent.
 * It also has a method for checking cell's traversability.
 * For its work is needed the size of agent and a map container that has 'cellIsObstacle' and 'cellOnGrid' methods.
 * If it is not possible to give the permission to access the grid, the one can use 'getCellsCrossedByLine' method.
 * It doesn't use grid and returns a set of all cells(as pairs of coordinates) that are crossed by an agent moving along a line.
 */

#ifndef LINEOFSIGHT_H
#define LINEOFSIGHT_H
#include "gl_const.h"

#define CN_OBSTACLE 1

#include <vector>
#include <unordered_set>
#include <assert.h>
#include <math.h>
#include <algorithm>

class LineOfSight
{
public:
    LineOfSight(double agentSize = 0.5)
    {
		extern bool homogenous;
		this->homogenous = homogenous;
		setSize(agentSize);
    }

    void setSize(double agentSize)
    {
		this->agentSize = agentSize;
		if (homogenous) {
			setSizeHomogenous(agentSize);
		}
		else {
			setSizeHeterogenous(agentSize);
		}
		if (cells.empty())
			cells.push_back({ 0,0 });
    }

    template <class T>
    std::vector<std::pair<int, int>> getCellsCrossedByLine(int x1, int y1, int x2, int y2, const T &map)
    {
        std::vector<std::pair<int, int>> lineCells_vector(0);
        if(x1 == x2 && y1 == y2)
        {
            for(auto cell:cells)
				lineCells_vector.push_back({x1+cell.first, y1+cell.second});
            return lineCells_vector;
        }
        int delta_x = std::abs(x1 - x2);
        int delta_y = std::abs(y1 - y2);
        if((delta_x >= delta_y && x1 > x2) || (delta_y > delta_x && y1 > y2))
        {
            std::swap(x1, x2);
            std::swap(y1, y2);
        }
        int step_x = (x1 < x2 ? 1 : -1);
        int step_y = (y1 < y2 ? 1 : -1);
        int error = 0, x = x1, y = y1;
        int k, num;
        std::pair<int, int> add;
        int gap = agentSize*sqrt(pow(delta_x, 2) + pow(delta_y, 2)) + double(delta_x + delta_y)/2 - CN_EPSILON;

        if(delta_x >= delta_y)
        {
            int extraCheck = agentSize*delta_y/sqrt(pow(delta_x, 2) + pow(delta_y, 2)) + 0.5 - CN_EPSILON;
            for(int n = 1; n <= extraCheck; n++)
            {
                error += delta_y;
                num = (gap - error)/delta_x;
                for(k = 1; k <= num; k++)
                    lineCells_vector.push_back({x1 - n*step_x, y1 + k*step_y});
                for(k = 1; k <= num; k++)
					lineCells_vector.push_back({x2 + n*step_x, y2 - k*step_y});
            }
            error = 0;
            for(x = x1; x != x2 + step_x; x+=step_x)
            {
				lineCells_vector.push_back({x, y});
                if(x < x2 - extraCheck)
                {
                    num = (gap + error)/delta_x;
                    for(k = 1; k <= num; k++)
						lineCells_vector.push_back({x, y + k*step_y});
                }
                if(x > x1 + extraCheck)
                {
                    num = (gap - error)/delta_x;
                    for(k = 1; k <= num; k++)
						lineCells_vector.push_back({x, y - k*step_y});
                }
                error += delta_y;
                if((error<<1) > delta_x)
                {
                    y += step_y;
                    error -= delta_x;
                }
            }
        }
        else
        {
            int extraCheck = agentSize*delta_x/sqrt(pow(delta_x, 2) + pow(delta_y, 2)) + 0.5 - CN_EPSILON;
            for(int n = 1; n <= extraCheck; n++)
            {
                error += delta_x;
                num = (gap - error)/delta_y;
                for(k = 1; k <= num; k++)
					lineCells_vector.push_back({x1 + k*step_x, y1 - n*step_y});
                for(k = 1; k <= num; k++)
					lineCells_vector.push_back({x2 - k*step_x, y2 + n*step_y});
            }
            error = 0;
            for(y = y1; y != y2 + step_y; y += step_y)
            {
				lineCells_vector.push_back({x, y});
                if(y < y2 - extraCheck)
                {
                    num = (gap + error)/delta_y;
                    for(k = 1; k <= num; k++)
						lineCells_vector.push_back({x + k*step_x, y});
                }
                if(y > y1 + extraCheck)
                {
                    num = (gap - error)/delta_y;
                    for(k = 1; k <= num; k++)
						lineCells_vector.push_back({x - k*step_x, y});
                }
                error += delta_x;
                if((error<<1) > delta_y)
                {
                    x += step_x;
                    error -= delta_y;
                }
            }
        }
        for(auto cell:cells)
        {
            add = {x1 + cell.first, y1 + cell.second};
			if (std::find(lineCells_vector.begin(), lineCells_vector.end(), add) == lineCells_vector.end()) {
				lineCells_vector.push_back(add);
			}
            add = {x2 + cell.first, y2 + cell.second};
			if (std::find(lineCells_vector.begin(), lineCells_vector.end(), add) == lineCells_vector.end()) {
				lineCells_vector.push_back(add);
			}
			
        }

        for(auto it = lineCells_vector.begin(); it != lineCells_vector.end(); it++)
            if(!map.CellOnGrid(it->first, it->second))
            {
				lineCells_vector.erase(it);
                it = lineCells_vector.begin();
            }
		//lineCells_vector.insert(lineCells_vector.end(), lineCells_set.begin(), lineCells_set.end());
        return lineCells_vector;
    }
    //returns all cells that are affected by agent during moving along a line

    template <class T>
    bool checkTraversability(int x, int y, const T &map)
    {
        for(int k = 0; k < cells.size(); k++)
            if(!map.CellOnGrid(x + cells[k].first, y + cells[k].second) || map.CellIsObstacle(x + cells[k].first, y + cells[k].second))
                return false;
        return true;
    }
    //checks traversability of all cells affected by agent's body

    template <class T>
    bool checkLine(int x1, int y1, int x2, int y2, const T &map)
    {
        //if(!checkTraversability(x1, y1) || !checkTraversability(x2, y2)) //additional check of start and goal traversability,
        //    return false;                                                //it can be removed if they are already checked

        int delta_x = std::abs(x1 - x2);
        int delta_y = std::abs(y1 - y2);
        if((delta_x > delta_y && x1 > x2) || (delta_y >= delta_x && y1 > y2))
        {
            std::swap(x1, x2);
            std::swap(y1, y2);
        }
        int step_x = (x1 < x2 ? 1 : -1);
        int step_y = (y1 < y2 ? 1 : -1);
        int error = 0, x = x1, y = y1;
        int gap = agentSize*sqrt(pow(delta_x, 2) + pow(delta_y, 2)) + double(delta_x + delta_y)/2 - CN_EPSILON;
        int k, num;

        if(delta_x > delta_y)
        {
            int extraCheck = agentSize*delta_y/sqrt(pow(delta_x, 2) + pow(delta_y, 2)) + 0.5 - CN_EPSILON;
            for(int n = 1; n <= extraCheck; n++)
            {
                error += delta_y;
                num = (gap - error)/delta_x;
                for(k = 1; k <= num; k++)
                    if(map.CellOnGrid(x1 - n*step_x, y1 + k*step_y))
                        if(map.CellIsObstacle(x1 - n*step_x, y1 + k*step_y))
                            return false;
                for(k = 1; k <= num; k++)
                    if(map.CellOnGrid(x2 + n*step_x, y2 - k*step_y))
                        if(map.CellIsObstacle(x2 + n*step_x, y2 - k*step_y))
                            return false;
            }
            error = 0;
            for(x = x1; x != x2 + step_x; x+=step_x)
            {
                if(map.CellIsObstacle(x, y))
                    return false;
                if(x < x2 - extraCheck)
                {
                    num = (gap + error)/delta_x;
                    for(k = 1; k <= num; k++)
                        if(map.CellIsObstacle(x, y + k*step_y))
                            return false;
                }
                if(x > x1 + extraCheck)
                {
                    num = (gap - error)/delta_x;
                    for(k = 1; k <= num; k++)
                        if(map.CellIsObstacle(x, y - k*step_y))
                            return false;
                }
                error += delta_y;
                if((error<<1) > delta_x)
                {
                    y += step_y;
                    error -= delta_x;
                }
            }
        }
        else
        {
            int extraCheck = agentSize*delta_x/sqrt(pow(delta_x, 2) + pow(delta_y, 2)) + 0.5 - CN_EPSILON;
            for(int n = 1; n <= extraCheck; n++)
            {
                error += delta_x;
                num = (gap - error)/delta_y;
                for(k = 1; k <= num; k++)
                    if(map.CellOnGrid(x1 + k*step_x, y1 - n*step_y))
                        if(map.CellIsObstacle(x1 + k*step_x, y1 - n*step_y))
                            return false;
                for(k = 1; k <= num; k++)
                    if(map.CellOnGrid(x2 - k*step_x, y2 + n*step_y))
                        if(map.CellIsObstacle(x2 - k*step_x, y2 + n*step_y))
                            return false;
            }
            error = 0;
            for(y = y1; y != y2 + step_y; y += step_y)
            {
                if(map.CellIsObstacle(x, y))
                    return false;
                if(y < y2 - extraCheck)
                {
                    num = (gap + error)/delta_y;
                    for(k = 1; k <= num; k++)
                        if(map.CellIsObstacle(x + k*step_x, y))
                            return false;
                }
                if(y > y1 + extraCheck)
                {
                    num = (gap - error)/delta_y;
                    for(k = 1; k <= num; k++)
                        if(map.CellIsObstacle(x - k*step_x, y))
                            return false;
                }
                error += delta_x;
                if((error<<1) > delta_y)
                {
                    x += step_x;
                    error -= delta_y;
                }
            }
        }
        return true;
    }
    //checks line-of-sight between a line
    std::vector<std::pair<int, int>> getCells(int i, int j)
    {
        std::vector<std::pair<int, int>> newcells;
        for(int k=0; k<this->cells.size(); k++)
			newcells.push_back({i+this->cells[k].first,j+this->cells[k].second});
        return newcells;
    }
private:
    double agentSize;
	bool homogenous;
    std::vector<std::pair<int, int>> cells; //cells that are affected by agent's body
	void setSizeHomogenous(double agentSize)
	{
		int x = agentSize, y = 0;

		// The initial point on the axes  
		// after translation
		cells.push_back({x,y});

		// When radius is zero only a single 
		// point will be printed 
		if (agentSize > 0)
		{
			cells.push_back({-x,0});
			cells.push_back({0,agentSize});
			cells.push_back({0,-agentSize});
		}

		// Initialising the value of P 
		int P = 1 - agentSize;
		while (x > y)
		{
			y++;

			// Mid-point is inside or on the perimeter 
			if (P <= 0)
				P = P + 2 * y + 1;
			// Mid-point is outside the perimeter 
			else
			{
				x--;
				P = P + 2 * y - 2 * x + 1;
			}

			// All the perimeter points have already been printed 
			if (x < y)
				break;

			// Printing the generated point and its reflection 
			// in the other octants after translation 
			cells.push_back({ x,y });
			cells.push_back({ -x,y });
			cells.push_back({ x,-y });
			cells.push_back({ -x,-y });

			// If the generated point is on the line x = y then  
			// the perimeter points have already been printed 
			if (x != y)
			{
				cells.push_back({ y,x });
				cells.push_back({ -y,x });
				cells.push_back({ y,-x });
				cells.push_back({ -y,-x });
			}
		}
	}

	void setSizeHeterogenous(double agentSize)
	{
		double sqDiameter = pow(2 * agentSize, 2);
		int add_x, add_y, num = agentSize + 0.5 - CN_EPSILON;
		cells.clear();
		for (int x = -num; x <= +num; x++)
			for (int y = -num; y <= +num; y++)
			{
				add_x = x != 0 ? 1 : 0;
				add_y = y != 0 ? 1 : 0;

				double pos = pow(2 * abs(x) - add_x, 2) + pow(2 * abs(y) - add_y, 2);
				if (pos < sqDiameter)
					cells.push_back({ x, y });
			}
	}

};

#endif // LINEOFSIGHT_H
