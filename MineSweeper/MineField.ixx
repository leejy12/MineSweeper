module;

#include <vector>
#include <random>
#include <ranges>
#include <algorithm>

export module MineField;

export struct CellInfo
{
    bool explored = false;
    bool hasMine = false;
    bool hasFlag = false;
    int adjacentMines = 0;
};

export class MineField
{
private:
    std::vector<std::vector<CellInfo>> _cells;
    int _fieldWidth, _fieldHeight;
    int _numMines;

public:
    MineField(int fieldWidth, int fieldHeight, int mines) :
        _fieldWidth(fieldWidth), _fieldHeight(fieldHeight), _numMines(mines)
    {
        _cells.resize(fieldWidth);
        for (auto& column : _cells)
            column.resize(fieldHeight);
    }

    void ToggleFlag(int x, int y)
    {
        _cells[x][y].hasFlag = !_cells[x][y].hasFlag;
    }

    void PlaceMines(int x, int y)
    {
        struct Index
        {
            int x, y;
        };

        // Randomly select `_numMines` number of cells that will contain mines
        std::vector<Index> indices;
        for (int y = 0; y < _fieldHeight; y++)
            for (int x = 0; x < _fieldWidth; x++)
                indices.push_back({ x, y });

        std::random_device rd;
        std::mt19937 eng(rd());
        std::ranges::shuffle(indices, eng);

        int k = 0;
        for (int i = 0; i < _numMines + k; i++)
        {
            if (indices[i].x == x && indices[i].y == y)
            {
                k = 1;
                continue;
            }
            _cells[indices[i].x][indices[i].y].hasMine = true;
        }

        // Calculate number of adjacent mines for each cell without a mine
        for (int x = 0; x < _fieldWidth; x++)
        {
            for (int y = 0; y < _fieldHeight; y++)
            {
                if (_cells[x][y].hasMine)
                    continue;

                if (x > 0)
                {
                    if (_cells[x - 1][y].hasMine)
                        _cells[x][y].adjacentMines++;
                    if (y > 0)
                        if (_cells[x - 1][y - 1].hasMine)
                            _cells[x][y].adjacentMines++;
                    if (y < _fieldHeight - 1)
                        if (_cells[x - 1][y + 1].hasMine)
                            _cells[x][y].adjacentMines++;
                }

                if (y > 0)
                    if (_cells[x][y - 1].hasMine)
                        _cells[x][y].adjacentMines++;
                if (y < _fieldHeight - 1)
                    if (_cells[x][y + 1].hasMine)
                        _cells[x][y].adjacentMines++;

                if (x < _fieldWidth - 1)
                {
                    if (_cells[x + 1][y].hasMine)
                        _cells[x][y].adjacentMines++;
                    if (y > 0)
                        if (_cells[x + 1][y - 1].hasMine)
                            _cells[x][y].adjacentMines++;
                    if (y < _fieldHeight - 1)
                        if (_cells[x + 1][y + 1].hasMine)
                            _cells[x][y].adjacentMines++;
                }
            }
        }
    }

    int StepOn(int x, int y)
    {
        if (_cells[x][y].explored || _cells[x][y].hasMine || _cells[x][y].hasFlag)
            return 0;

        _cells[x][y].explored = true;

        if (_cells[x][y].adjacentMines > 0)
            return 1;

        int exploredCells = 0;

        if (x > 0)
        {
            exploredCells += StepOn(x - 1, y);
            if (y > 0)
                exploredCells += StepOn(x - 1, y - 1);
            if (y < _fieldHeight - 1)
                exploredCells += StepOn(x - 1, y + 1);
        }

        if (y > 0)
            exploredCells += StepOn(x, y - 1);
        if (y < _fieldHeight - 1)
            exploredCells += StepOn(x, y + 1);

        if (x < _fieldWidth - 1)
        {
            exploredCells += StepOn(x + 1, y);
            if (y > 0)
                exploredCells += StepOn(x + 1, y - 1);
            if (y < _fieldHeight - 1)
                exploredCells += StepOn(x + 1, y + 1);
        }

        // The 1 that is added here corresponds to the current cell (x, y)
        return 1 + exploredCells;
    }

    void Reset(int newWidth, int newHeight, int newMines)
    {
        _fieldWidth = newWidth;
        _fieldHeight = newHeight;
        _numMines = newMines;

        _cells.clear();
        _cells.resize(newWidth);
        for (auto& column : _cells)
            column.resize(newHeight);
    }

    CellInfo GetCellInfo(int x, int y) const
    {
        return _cells[x][y];
    }

    int GetWidth() const
    {
        return _fieldWidth;
    }

    int GetHeight() const
    {
        return _fieldHeight;
    }

    int GetNumMines() const
    {
        return _numMines;
    }
};
