#ifndef __WAVE_FUNCTION_COLLAPSE__
#define __WAVE_FUNCTION_COLLAPSE__

#include "WfcCell.hpp"
#include <vector>
#include <iostream>
#include <map>

template<int WIDTH, int HEIGHT>
class WaveFunctionCollapse
{
public:
    WaveFunctionCollapse(int** paterne,int sizeX,int sizeY);
    void step();
    void generate();
    bool is_solved();
    void print();
    void print_proba();
    WfcCell * get_cell();
private:
    void UpdateInversePosibility(std::vector<int>& posibility, Direction d, std::vector<int>& all_posibility);
    void WfcUpdateNeighbor(int index);
private:
    WfcCell m_arr[WIDTH * HEIGHT];
    int** m_paterne;
    int m_sizeX;
    int m_sizeY;
    std::vector<int> m_allPosibility;
    std::map<int, std::map<Direction, std::vector<int>>> map_possibility;
};

template<int WIDTH, int HEIGHT>
WaveFunctionCollapse<WIDTH, HEIGHT>::WaveFunctionCollapse(int** paterne, int sizeX, int sizeY)
{
    srand(time(NULL));
    m_paterne = paterne;
    m_sizeX = sizeX;
    m_sizeY = sizeY;    
    for (int x = 0; x < m_sizeX; x++) 
    {
        for (int y = 0; y < m_sizeY; y++) 
        {
            int valeur = m_paterne[x][y];
            m_allPosibility.push_back(valeur);
            if (map_possibility.find(valeur) == map_possibility.end()) 
            {
                map_possibility[valeur] = {};
            }

            // Coordonnées des voisins
            int voisinsX[8] = { x, x, x - 1, x + 1,x - 1,x + 1, x - 1,x + 1 };
            int voisinsY[8] = { y - 1, y + 1, y, y,y - 1,y - 1, y + 1,y + 1 };

            for (int i = 0; i < 8; i++) 
            {
                if (voisinsX[i] >= 0 && voisinsX[i] < m_sizeX && voisinsY[i] >= 0 && voisinsY[i] < m_sizeY) 
                {
                    map_possibility[valeur][static_cast<Direction>(i)].push_back(m_paterne[voisinsX[i]][voisinsY[i]]);
                }
            }
        }
    }
    
    int count_entropy = 0;
    std::unordered_set<int> uniqueElements;
    for (int i : m_allPosibility)
    {
        if (uniqueElements.insert(i).second)
        {
            count_entropy++;
        }
    }    

    for (int i = 0; i < WIDTH * HEIGHT; i++)
    {
        this->m_arr[i] = WfcCell(m_allPosibility, count_entropy);
    }

    /*for (const auto& entry : map_possibility)
    {
        int key = entry.first;
        std::cout << "Key: " << key << std::endl;
    
        for (const auto& directionEntry : entry.second) 
        {
            Direction direction = directionEntry.first;
            std::cout << "  Direction: " << direction << std::endl;
        
            const std::vector<int>& values = directionEntry.second;
            for (const auto& value : values) 
            {
                std::cout << value << ",";
            }
            std::cout << std::endl;
        }
    }*/
}

template<int WIDTH, int HEIGHT>
void WaveFunctionCollapse<WIDTH, HEIGHT>::UpdateInversePosibility(std::vector<int>& posibility, Direction d, std::vector<int> & all_posibility)
{    
    if (posibility.size() == m_allPosibility.size())
    {
        return;
    }
    std::vector<int> local_posibility;
    std::vector<int> temp_posibility;
    for (int i = 0; i < posibility.size(); i++)
    {
        temp_posibility = map_possibility[posibility[i]][d];
        local_posibility.insert(local_posibility.end(), temp_posibility.begin(), temp_posibility.end());
    }
    std::sort(local_posibility.begin(), local_posibility.end());
    std::sort(all_posibility.begin(), all_posibility.end());
    std::vector<int> nv;
    std::set_intersection(all_posibility.begin(), all_posibility.end(),
        local_posibility.begin(), local_posibility.end(),
        std::back_inserter(nv));
    all_posibility = nv;
}

template<int WIDTH, int HEIGHT>
void WaveFunctionCollapse<WIDTH, HEIGHT>::WfcUpdateNeighbor(int index)
{
    if (index >= 0 && index < WIDTH * HEIGHT)
    {
        if (this->m_arr[index].is_collapsed())
        {
            return;
        }
        std::vector<int> & all_posibility = this->m_arr[index].getPossibilites();        
        
        if (index - WIDTH >= 0)
        {
             UpdateInversePosibility(this->m_arr[index - WIDTH].getPossibilites(), Direction::DOWN, all_posibility);
        }

        if (index + WIDTH < WIDTH * HEIGHT)
        {        
             UpdateInversePosibility(this->m_arr[index + WIDTH].getPossibilites(), Direction::UP, all_posibility);
        }

        if (index - 1 >= 0 && index / WIDTH == (index - 1) / WIDTH)
        {
             UpdateInversePosibility(this->m_arr[index - 1].getPossibilites(), Direction::RIGHT, all_posibility);
        }

        if (index + 1 < WIDTH * HEIGHT && index / WIDTH == (index + 1) / WIDTH)
        {
             UpdateInversePosibility(this->m_arr[index + 1].getPossibilites(), Direction::LEFT, all_posibility);
        }

        if (index - WIDTH - 1 >= 0 && (index - WIDTH) / WIDTH == (index - WIDTH - 1) / WIDTH)
        {
              UpdateInversePosibility(this->m_arr[index - WIDTH - 1].getPossibilites(), Direction::DOWNRIGHT, all_posibility);
        }

        if (index - WIDTH + 1 >= 0 && (index - WIDTH) / WIDTH == (index - WIDTH + 1) / WIDTH)
        {
             UpdateInversePosibility(this->m_arr[index - WIDTH + 1].getPossibilites(), Direction::DOWNLEFT, all_posibility);
        }

        if (index + WIDTH - 1 < WIDTH * HEIGHT && (index + WIDTH) / WIDTH == (index + WIDTH - 1) / WIDTH)
        {
             UpdateInversePosibility(this->m_arr[index + WIDTH - 1].getPossibilites(), Direction::UPRIGHT, all_posibility);
        }

        if (index + WIDTH + 1 < WIDTH * HEIGHT && (index + WIDTH) / WIDTH == (index + WIDTH + 1) / WIDTH)
        {
             UpdateInversePosibility(this->m_arr[index + WIDTH + 1].getPossibilites(), Direction::UPLEFT, all_posibility);
        }
        this->m_arr[index].computeEntropy();
    }
}

template<int WIDTH, int HEIGHT>
void WaveFunctionCollapse<WIDTH, HEIGHT>::step()
{
    std::vector<int> lowest_entropy;

    int current_lowest = INT32_MAX;

    for (int i = 0; i < WIDTH * HEIGHT; i++)
    {
        int cell_entropy = this->m_arr[i].get_entropy();

        if (this->m_arr[i].is_collapsed())
        {
            continue;
        }
        else if (cell_entropy == current_lowest)
        {
            lowest_entropy.push_back(i);
        }
        else if (cell_entropy < current_lowest)
        {
            lowest_entropy.clear();
            lowest_entropy.push_back(i);
            current_lowest = cell_entropy;
        }
    }
    int index;
    int tile;
    if (lowest_entropy.size() > 0)
    {
        index = lowest_entropy[std::rand() % lowest_entropy.size()];

        this->m_arr[index].collapse();

        tile = this->m_arr[index].get_value();
    }
    
    {
        for (int i = 0; i < WIDTH * HEIGHT; i++)
        {
            WfcUpdateNeighbor(i);
        }
    }
}

template<int WIDTH, int HEIGHT>
bool WaveFunctionCollapse<WIDTH, HEIGHT>::is_solved()
{
    bool solved = true;

    for (int i = 0; i < WIDTH * HEIGHT; i++)
    {
        if (!this->m_arr[i].is_collapsed())
        {
            solved = false;
            break;
        }
    }

    return solved;
}

template<int WIDTH, int HEIGHT>
void WaveFunctionCollapse<WIDTH, HEIGHT>::generate()
{    
    while (!this->is_solved())
    {                
        this->step();
        print_proba();
    }
}

template<int WIDTH, int HEIGHT>
WfcCell* WaveFunctionCollapse<WIDTH, HEIGHT>::get_cell()
{
    return this->m_arr;
}

template<int WIDTH, int HEIGHT>
void WaveFunctionCollapse<WIDTH, HEIGHT>::print()
{    
    int val;
    for (int i = 0; i < WIDTH * HEIGHT; i++)
    {
        if (i % WIDTH == 0 && i != 0)
        {
            std::cout << std::endl;
        }
        val = m_arr[i].get_value();
        if (val == -1)
        {
            std::cout << "#,";
        }
        else if (val == -2)
        {
                std::cout << "?,";
        }
        else
        {
            std::cout << val << ",";
        }
    }
    std::cout << std::endl << std::endl;
}


template<int WIDTH, int HEIGHT>
void WaveFunctionCollapse<WIDTH, HEIGHT>::print_proba()
{
    int val;
    for (int i = 0; i < WIDTH * HEIGHT; i++)
    {
        if (i % WIDTH == 0 && i != 0)
        {
            std::cout << std::endl;
        }
        val = m_arr[i].get_entropy();
        if (val < 10)
        {
            std::cout << "0" << val << ",";
        }
        else
        {
            std::cout << val << ",";
        }
    }
    std::cout << std::endl << std::endl;
}

#endif //!__WAVE_FUNCTION_COLLAPSE__