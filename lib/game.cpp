//
// Created by admin on 25-3-30.
//

#include "game.h"

vector<int> getTeamNumbers(int n, int gameIndex)
{
    static vector<vector<int>> tables;
    static int numbers;
    if (numbers == n)
    {
        return tables[gameIndex];
    }
    numbers = n;
    tables.clear();
    for (int i = 1; i <= n - 1; i++)
    {
        for (int j = i + 1; j <= n; j++)
        {
            tables.push_back({i, j});
        }
    }
    return tables[gameIndex];
}

int getGameNumbers(int n)
{
    int sum = 0;
    for (int i = n - 1; i >= 1; i--)
    {
        sum += i;
    }
    return sum;
}

void dfs(set<vector<int>>& res, vector<int> teamScore, int n, int gameIndex, int win, int loss, int ban,
         bool isBan = true)
{
    if (getGameNumbers(n) == gameIndex)
    {
        sort(teamScore.begin(), teamScore.end(), [](int x1, int x2)
        {
            return x1 >= x2;
        });
        res.insert(teamScore);
        return;
    }
    auto twoTeams = getTeamNumbers(n, gameIndex);
    int team1 = twoTeams[0];
    int team2 = twoTeams[1];

    // 1 win
    teamScore[team1 - 1] += win;
    teamScore[team2 - 1] += loss;
    dfs(res, teamScore, n, gameIndex + 1, win, loss, ban, isBan);
    teamScore[team1 - 1] -= win;
    teamScore[team2 - 1] -= loss;

    // 2 win
    teamScore[team2 - 1] += win;
    teamScore[team1 - 1] += loss;
    dfs(res, teamScore, n, gameIndex + 1, win, loss, ban, isBan);
    teamScore[team2 - 1] -= win;
    teamScore[team1 - 1] -= loss;

    // ban
    if (isBan)
    {
        teamScore[team1 - 1] += 1;
        teamScore[team2 - 1] += 1;
        dfs(res, teamScore, n, gameIndex + 1, win, loss, ban, isBan);
        teamScore[team1 - 1] -= 1;
        teamScore[team2 - 1] -= 1;
    }
}

set<vector<int>> games(int n, int win, int loss, int ban, bool isBan)
{
    set<vector<int>> res;
    vector<int> score(n, 0);
    dfs(res, score, n, 0, win, loss, ban, isBan);
    cout << res.size() << endl;
    return res;
}
