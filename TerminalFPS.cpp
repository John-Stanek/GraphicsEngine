#include <iostream>
#include <Windows.h>
#include <cmath>
#include <chrono>
#include <vector>
#include <algorithm>

using namespace std;

int screenWidth = 120;
int screenHeight = 40;

float playerX = 2.0f;
float playerY = 2.0f;
float playerA = 0.0f;

int mapHeight = 16;
int mapWidth = 16;

float FOV = 3.14159 / 4;
float fdepth = 16.0f;
float speed = 5.0f;
float pi = 3.14159f;

int main(int argc, char const *argv[])
{
    wchar_t *screen = new wchar_t[screenWidth * screenHeight];
    HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
    SetConsoleActiveScreenBuffer(hConsole);
    DWORD dwBytesWritten = 0;

    wstring map;

    // Y ^
    // X <->
    // Angle of 0 looks Down
    // Angle of 90 degrees looks -->

    map += L"#########.......";
    map += L"#...............";
    map += L"#.......########";
    map += L"#..............#";
    map += L"#......##......#";
    map += L"#......##......#";
    map += L"#..............#";
    map += L"###............#";
    map += L"##.............#";
    map += L"#......####..###";
    map += L"#......#.......#";
    map += L"#......#.......#";
    map += L"#..............#";
    map += L"#......#########";
    map += L"#..............#";
    map += L"################";

    auto tp1 = chrono::system_clock::now();
    auto tp2 = chrono::system_clock::now();

    while (true)
    {

        tp2 = chrono::system_clock::now();
        chrono::duration<float> elapsedTime = tp2 - tp1;
        tp1 = tp2;
        float fElapsedTime = elapsedTime.count();

        // Controls
        // Handle CCW Rotation
        if (GetAsyncKeyState((unsigned short)'A') & 0x8000)
            playerA -= (1.0f) * fElapsedTime;

        // Handle CW Rotation
        if (GetAsyncKeyState((unsigned short)'D') & 0x8000)
            playerA += (1.0f) * fElapsedTime;

        // Handle Foward movement
        if (GetAsyncKeyState((unsigned short)'W') & 0x8000)
        {
            playerX += sinf(playerA) * (speed)*fElapsedTime;
            playerY += cosf(playerA) * (speed)*fElapsedTime;

            // Collision Detection
            if (map[(int)playerY * mapWidth + (int)playerX] == '#')
            {
                playerX -= sinf(playerA) * (speed)*fElapsedTime;
                playerY -= cosf(playerA) * (speed)*fElapsedTime;
            }
        }

        // Handle Backward movement
        if (GetAsyncKeyState((unsigned short)'S') & 0x8000)
        {

            playerX -= sinf(playerA) * (speed)*fElapsedTime;
            playerY -= cosf(playerA) * (speed)*fElapsedTime;

            // Collision Detection
            if (map[(int)playerY * mapWidth + (int)playerX] == '#')
            {
                playerX += sinf(playerA) * (speed)*fElapsedTime;
                playerY += cosf(playerA) * (speed)*fElapsedTime;
            }
        }

        // Handle Strafe Right movement
        if (GetAsyncKeyState((unsigned short)'E') & 0x8000)
        {
            playerX += sinf(playerA + pi / 2) * (speed / 2) * fElapsedTime;
            playerY += cosf(playerA + pi / 2) * (speed / 2) * fElapsedTime;

            // Collision Detection
            if (map[(int)playerY * mapWidth + (int)playerX] == '#')
            {
                playerX -= sinf(playerA + pi / 2) * (speed / 2) * fElapsedTime;
                playerY -= cosf(playerA + pi / 2) * (speed / 2) * fElapsedTime;
            }
        }

        // Handle Strafe Left movement
        if (GetAsyncKeyState((unsigned short)'Q') & 0x8000)
        {
            playerX += sinf(playerA - pi / 2) * (speed / 2) * fElapsedTime;
            playerY += cosf(playerA - pi / 2) * (speed / 2) * fElapsedTime;

            // Collision Detection
            if (map[(int)playerY * mapWidth + (int)playerX] == '#')
            {
                playerX -= sinf(playerA - pi / 2) * (speed / 2) * fElapsedTime;
                playerY -= cosf(playerA - pi / 2) * (speed / 2) * fElapsedTime;
            }
        }

        for (int x = 0; x < screenWidth; x++)
        {
            // for each column, calculate the projected ray angle into world space
            float rayAngle = (playerA - FOV / 2.0f) + ((float)x / (float)screenWidth) * FOV;

            float stepSize = 0.1f; // increment size for ray casting, decrease to increase resolution
            float fDistanceToWall = 0.0f;

            bool hitwall = false;
            bool hitbound = false;

            float fEyeX = sinf(rayAngle); // unit vector for ray in player space
            float fEyeY = cosf(rayAngle);

            while (!hitwall && fDistanceToWall <= fdepth)
            {
                fDistanceToWall += stepSize;

                int nTestX = (int)(playerX + fEyeX * fDistanceToWall);
                int nTestY = (int)(playerY + fEyeY * fDistanceToWall);

                // Test if ray is out of bounds
                if (nTestX < 0 || nTestX >= mapWidth || nTestY < 0 || nTestY >= mapHeight)
                {
                    hitwall = true;
                    // Set distance to maximum depth
                    fDistanceToWall = fdepth;
                }
                else
                {
                    // Ray is inbounds so test to see if the ray cell is a wall block
                    if (map.c_str()[nTestY * mapWidth + nTestX] == '#')
                    {
                        hitwall = true;

                        // check to see if corner of wall block is hit
                        vector<pair<float, float>> p; // distance,dot product

                        for (int tx = 0; tx < 2; tx++)
                        {
                            for (int ty = 0; ty < 2; ty++)
                            {
                                // vector to corners
                                float vy = (float)nTestY + ty - playerY;
                                float vx = (float)nTestX + tx - playerX;
                                float d = sqrt(vx * vx + vy * vy);
                                float dot = (fEyeX * vx / d) + (fEyeY * vy / d);
                                p.push_back(make_pair(d, dot));
                            }
                        }
                        // Sort Pairs from closest to farthest
                        sort(p.begin(), p.end(), [](const pair<float, float> &left, const pair<float, float> &right)
                             { return left.first < right.first; });

                        float fBound = 0.01;
                        if (acos(p.at(0).second) < fBound)
                            hitbound = true;
                        if (acos(p.at(1).second) < fBound)
                            hitbound = true;
                        // if (acos(p.at(2).second) < fBound)
                        //     hitbound = true;
                    }
                }
            }

            // Calculate distance to ceiling and floor
            int nCeiling = (float)(screenHeight / 2.0) - screenHeight / ((float)fDistanceToWall);
            int nFloor = screenHeight - nCeiling;

            short nShade = ' ';

            if (fDistanceToWall <= fdepth / 4.0f)
                nShade = 0x2588; // Very close
            else if (fDistanceToWall < fdepth / 3.0f)
                nShade = 0x2593;
            else if (fDistanceToWall < fdepth / 2.0f)
                nShade = 0x2592;
            else if (fDistanceToWall < fdepth)
                nShade = 0x2591;
            else
                nShade = ' '; // Far Away

            if (hitbound)
                nShade = ' '; // black out boundary

            for (int y = 0; y < screenHeight; y++)
            {
                if (y < nCeiling) // Shade Ceiling
                {
                    screen[y * screenWidth + x] = ' ';
                }
                else if (y > nCeiling && y <= nFloor)
                {
                    screen[y * screenWidth + x] = nShade;
                }
                else // Shade floor
                {
                    float b = 1.0f - (((float)y - screenHeight / 2.0f) / ((float)screenHeight / 2.0f));
                    if (b < 0.25) // Close
                        nShade = '#';
                    else if (b < 0.5)
                        nShade = 'x';
                    else if (b < 0.75)
                        nShade = '.';
                    else if (b < 0.9)
                        nShade = '-';
                    // else // Far
                    //     nShade = ' ';

                    screen[y * screenWidth + x] = nShade;
                }
            }
        }
        // Display Stats
        swprintf_s(screen, 40, L"X=%3.2f, Y=%3.2f,A=%3.2f FPS=%3.2f ", playerX, playerY, playerA, 1.0f / fElapsedTime);

        // Display Map
        for (int nx = 0; nx < mapWidth; nx++)
            for (int ny = 0; ny <= mapHeight; ny++)
            {
                screen[(ny + 1) * screenWidth + nx] = map[ny * mapWidth - nx - 1];
            }
        screen[((int)playerY + 2) * screenWidth + (int)(mapWidth - playerX)] = 'P';

        // Direction, convert to degrees to use modulo

        int playerADegree = (int)(playerA * 180 / pi) % 360;
        if (playerA < 0)
        {
            playerADegree = -(int)(playerA * 180 / pi) % 360;
        }

        if (45 < playerADegree && playerADegree < 135 && playerX > 0 && playerY < mapHeight - 1)
        {
            screen[((int)playerY + 3) * screenWidth + (int)(mapWidth - playerX - 1)] = '/';
        }

        if (135 < playerADegree && playerADegree < 225 && playerX > 0 && playerY > 0)
        {
            screen[((int)playerY + 1) * screenWidth + (int)(mapWidth - playerX - 1)] = '\\';
        }
        if (225 < playerADegree && playerADegree < 315 && playerX < mapWidth - 1 && playerY > 0)
        {
            screen[((int)playerY + 1) * screenWidth + (int)(mapWidth - playerX + 1)] = '/';
        }

        if (((0 < playerADegree && playerADegree < 45) || (315 < playerADegree && playerADegree < 360)) && playerY < mapHeight + 1 && playerX < mapWidth - 1)
        {
            screen[((int)playerY + 3) * screenWidth + (int)(mapWidth - playerX + 1)] = '\\';
        }

        screen[screenWidth * screenHeight - 1] = '\0';
        WriteConsoleOutputCharacterW(hConsole, screen, screenWidth * screenHeight, {0, 0}, &dwBytesWritten);
    }
    return 0;
}
