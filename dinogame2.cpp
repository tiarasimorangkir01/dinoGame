#include <iostream>
#include <conio.h>
#include <time.h>
#include <windows.h>
#include <string>
#include <algorithm>
#include <map>
#include <queue>
#include <vector>
#include <fstream>

#define SCORE_FILE "scores_dino.txt"
#define DINO_POS_X 2
#define HURDLE_START_X 70
#define MAX_SCORE_HISTORY 100

using namespace std;

HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);

// ==================== PROTOTYPE FUNGSI (untuk mengatasi error scope) ====================
void showLeaderboard();
void searchPlayer();
void showJalur();
void play();
void instructions();
void saveScoreToFile(string name,int score);
void loadScoresFromFile();

// ==================== GAME STATE ====================
int dinoY = 0;
int speed = 40;
int gameover = 0;
int currentScore = 0;
string playerName = "Player";

// ==================== QUEUE (Pertemuan 5) - Untuk Rintangan ====================
struct Hurdle {
    int x;
    int y;
    bool passed;
};

class QueueHurdle {
private:
    vector<Hurdle> arr;
    int front;
    
public:
    QueueHurdle() {
        front = 0;
    }
    
    void enqueue(Hurdle h) {
        arr.push_back(h);
    }
    
    Hurdle dequeue() {
        if (isEmpty()) {
            return Hurdle();
        }
        Hurdle h = arr[front];
        front++;
        return h;
    }
    
    bool isEmpty() {
        return front >= arr.size();
    }
    
    Hurdle getFront() {
        if (isEmpty()) return Hurdle();
        return arr[front];
    }
	
	Hurdle* getFrontRef() {
    if (isEmpty()) return nullptr;
    return &arr[front];
	}
    
    int size() {
        return arr.size() - front;
    }
};

QueueHurdle hurdleQueue;

// ==================== STACK (Pertemuan 4) - Untuk Riwayat Skor ====================
class StackScore {
private:
    int arr[MAX_SCORE_HISTORY];
    int top;
    
public:
    StackScore() {
        top = -1;
    }
    
    void push(int score) {
        if (top < MAX_SCORE_HISTORY - 1) {
            top++;
            arr[top] = score;
        }
    }
    
    int pop() {
        if (top == -1) return -1;
        int score = arr[top];
        top--;
        return score;
    }
    
    bool isEmpty() {
        return top == -1;
    }
    
    int getTop() {
        if (top == -1) return -1;
        return arr[top];
    }
};

StackScore scoreHistory;

// ==================== BST (Pertemuan 9) - Untuk Leaderboard ====================
struct PlayerNode {
    string name;
    int score;
    PlayerNode* left;
    PlayerNode* right;
};

class BSTLeaderboard {
private:
    PlayerNode* root;
    
    PlayerNode* insertNode(PlayerNode* node, string name, int score) {
        if (node == nullptr) {
            node = new PlayerNode;
            node->name = name;
            node->score = score;
            node->left = nullptr;
            node->right = nullptr;
        } else if (score <= node->score) {
            node->left = insertNode(node->left, name, score);
        } else {
            node->right = insertNode(node->right, name, score);
        }
        return node;
    }
    
    void inorderTraversal(PlayerNode* node, vector<pair<string,int>>& result) {
        if (node != nullptr) {
            inorderTraversal(node->left, result);
            result.push_back({node->name, node->score});
            inorderTraversal(node->right, result);
        }
    }
    
public:
    BSTLeaderboard() {
        root = nullptr;
    }
    
    void insertPlayer(string name, int score) {
        root = insertNode(root, name, score);
    }
    
    vector<pair<string,int>> getLeaderboardSorted() {
        vector<pair<string,int>> result;
        inorderTraversal(root, result);
        sort(result.begin(), result.end(), [](pair<string,int> a, pair<string,int> b) {
            return a.second > b.second;
        });
        return result;
    }
};

BSTLeaderboard leaderboard;

// ==================== HASHING (Pertemuan 13) - Untuk Pencarian Pemain ====================
class HashTablePlayer {
private:
    map<string, int> hashTable;
    
public:
    void insertPlayer(string name, int score) {
        hashTable[name] = score;
    }
    
    int searchPlayer(string name) {
        if (hashTable.find(name) != hashTable.end()) {
            return hashTable[name];
        }
        return -1;
    }
    
    bool hasPlayer(string name) {
        return hashTable.find(name) != hashTable.end();
    }
    
    void displayAll() {
        for (auto& p : hashTable) {
            cout << p.first << " - " << p.second << endl;
        }
    }
};

HashTablePlayer playerHash;

// ==================== GRAPH + BFS/DFS (Pertemuan 11 & 12) - Untuk Jalur ====================
class Graph {
private:
    int V;
    map<int, vector<int>> adj;
    
public:
    Graph(int vertices) {
        V = vertices;
    }
    
    void addEdge(int u, int v) {
        adj[u].push_back(v);
        adj[v].push_back(u);
    }
    
    vector<int> BFS(int start, int end) {
        vector<int> path;
        map<int, bool> visited;
        map<int, int> parent;
        
        queue<int> q;
        q.push(start);
        visited[start] = true;
        parent[start] = -1;
        
        while (!q.empty()) {
            int u = q.front();
            q.pop();
            
            if (u == end) break;
            
            for (int v : adj[u]) {
                if (!visited[v]) {
                    visited[v] = true;
                    parent[v] = u;
                    q.push(v);
                }
            }
        }
        
        if (visited[end]) {
            int curr = end;
            while (curr != -1) {
                path.push_back(curr);
                curr = parent[curr];
            }
            reverse(path.begin(), path.end());
        }
        
        return path;
    }
};

// ==================== HELPER FUNCTIONS ====================
void gotoxy(int x, int y) {
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

void setcursor(bool visible, DWORD size) {
    if (size == 0) size = 20;
    CONSOLE_CURSOR_INFO lpCursor;
    lpCursor.bVisible = visible;
    lpCursor.dwSize = size;
    SetConsoleCursorInfo(console, &lpCursor);
}

// ==================== INITIALISASI GAME ====================
void init() {
    system("cls");
    gameover = 0;
    currentScore = 0;
    dinoY = 0;
    speed = 40;
    
    scoreHistory.push(0);
    
    gotoxy(3, 2); cout << "PLAYER: " << playerName;
    gotoxy(3, 3); cout << "SCORE : ";
    
    for (int i = 0; i < 79; i++) {
        gotoxy(1 + i, 1); cout << "п";
        gotoxy(1 + i, 25); cout << "п";
    }
    
    for (int i = 0; i < 1; i++) {
        Hurdle h;
        h.x = HURDLE_START_X - (i * 40);
        h.y = 20;
        h.passed = false;
        hurdleQueue.enqueue(h);
    }
}

// ==================== MOVE DINO ====================
void moveDino(int jump = 0) {
    static int foot = 0;

    // Hapus HANYA baris terakhir dino sebelumnya (bukan seluruh area)
    gotoxy(DINO_POS_X, 15 - dinoY); cout << "                    ";
    gotoxy(DINO_POS_X, 16 - dinoY); cout << "                    ";
    gotoxy(DINO_POS_X, 17 - dinoY); cout << "                    ";
    gotoxy(DINO_POS_X, 18 - dinoY); cout << "                    ";
    gotoxy(DINO_POS_X, 19 - dinoY); cout << "                    ";
    gotoxy(DINO_POS_X, 20 - dinoY); cout << "                    ";
    gotoxy(DINO_POS_X, 21 - dinoY); cout << "                    ";
    gotoxy(DINO_POS_X, 22 - dinoY); cout << "                    ";
    gotoxy(DINO_POS_X, 23 - dinoY); cout << "                    ";
    gotoxy(DINO_POS_X, 24 - dinoY); cout << "                    ";

    // Baru update posisi
    if (jump == 0)
        dinoY = 0;
    else if (jump == 2)
        dinoY--;
    else dinoY++;

    // Gambar dino di posisi baru
    gotoxy(DINO_POS_X, 16 - dinoY); cout << "         млпллллм";
    gotoxy(DINO_POS_X, 17 - dinoY); cout << "         лллллллл";
    gotoxy(DINO_POS_X, 18 - dinoY); cout << "         лллллппп";
    gotoxy(DINO_POS_X, 19 - dinoY); cout << " л      мллллппп ";
    gotoxy(DINO_POS_X, 20 - dinoY); cout << " ллм  мллллллммм ";
    gotoxy(DINO_POS_X, 21 - dinoY); cout << " пллллллллллл  п ";
    gotoxy(DINO_POS_X, 22 - dinoY); cout << "   плллллллп     ";
    gotoxy(DINO_POS_X, 23 - dinoY);

    if (jump == 1 || jump == 2) {
        cout << "    ллп пл       ";
        gotoxy(2, 24 - dinoY);
        cout << "    лм   лм      ";
    } else if (foot == 0) {
        cout << "    пллп  ппп    ";
        gotoxy(2, 24 - dinoY);
        cout << "      лм         ";
        foot = !foot;
    } else if (foot == 1) {
        cout << "     плм пл      ";
        gotoxy(2, 24 - dinoY);
        cout << "          лм     ";
        foot = !foot;
    }

    // Selalu gambar ulang ground
    gotoxy(1, 25);
    for (int i = 0; i < 79; i++) cout << "п";

    Sleep(speed);
}
// ==================== DRAW HURDLE (INTEGRASI QUEUE) ====================
void drawHurdle() {
    // Spawn hurdle baru jika queue kosong
    if (hurdleQueue.isEmpty()) {
        Hurdle h;
        h.x = HURDLE_START_X;
        h.y = 20;
        h.passed = false;
        hurdleQueue.enqueue(h);
    }

    Hurdle* current = hurdleQueue.getFrontRef();
    if (current == nullptr) return;

    // Hapus gambar hurdle di posisi lama
    for (int y = 20; y <= 24; y++) {
        gotoxy(current->x, y);
        cout << "      ";
    }

    // Gerakkan hurdle ke kiri
    current->x--;

    // Gambar hurdle di posisi baru
	if (current->x > 0 && current->x < 75){
    gotoxy(current->x, 20); cout << "| | ";
    gotoxy(current->x, 21); cout << "| | ";
    gotoxy(current->x, 22); cout << "|_| ";
    gotoxy(current->x, 23); cout << " |  ";
    gotoxy(current->x, 24); cout << " |  ";
	}
    // Deteksi tabrakan
    if (current->x == DINO_POS_X + 5 && dinoY < 4) {
        gotoxy(36, 8);  cout << "Game Over";
        gotoxy(36, 10); cout << "Final Score: " << currentScore;
        gameover = 1;
        getch();
        return;
    }

    // Hurdle sudah melewati dino ? tambah skor & spawn baru
    if (current->x < DINO_POS_X - 5 && !current->passed) {
        current->passed = true;
        currentScore++;
        scoreHistory.push(currentScore);

        gotoxy(11, 2); cout << "     ";
        gotoxy(11, 2); cout << currentScore;

        if (speed > 20) speed--;

        // Hapus hurdle lama, spawn hurdle baru
        hurdleQueue.dequeue();

        Hurdle h;
        h.x = HURDLE_START_X;
        h.y = 20;
        h.passed = false;
        hurdleQueue.enqueue(h);
    }
}
// ==================== PLAY GAME ====================
void play() {
    system("cls");
    char ch;
    int i;
    
    init();
    
    while (true) {
        while (!kbhit()) {
				if (gameover == 1) {
		leaderboard.insertPlayer(playerName, currentScore);
		playerHash.insertPlayer(playerName, currentScore);
		saveScoreToFile(playerName, currentScore);
		return;
}
            moveDino();
            drawHurdle();
        }
        
        ch = getch();
        if (ch == 32) {
            i = 0;
            while (i < 12) {
                moveDino(1);
                drawHurdle();
                i++;
            }
			 while (i > 0) {
				moveDino(2);
				drawHurdle();
				i--;
    }
			
        } else if (ch == 'p' || ch == 'P') {
            getch();
        } else if (ch == 27) {
            break;
        } else if (ch == 'r' || ch == 'R') {
            if (!scoreHistory.isEmpty()) {
                currentScore = scoreHistory.pop();
                gotoxy(11, 2); cout << "     ";
                gotoxy(11, 2); cout << currentScore;
            }
        } else if (ch == 'l' || ch == 'L') {
            showLeaderboard();
        } else if (ch == 's' || ch == 'S') {
            searchPlayer();
        } else if (ch == 'j' || ch == 'J') {
            showJalur();
        }
    }
}

// ==================== SHOW LEADERBOARD ====================
void showLeaderboard() {
    system("cls");
    cout << "\n========== LEADERBOARD ==========\n";
    
    vector<pair<string,int>> board = leaderboard.getLeaderboardSorted();
    
    if (board.empty()) {
        cout << "Belum ada pemain!\n";
    } else {
        for (int i = 0; i < board.size() && i < 10; i++) {
            cout << i + 1 << ". " << board[i].first << " - " << board[i].second << " poin\n";
        }
    }
    
    cout << "\nPress any key to go back\n";
    getch();
}

// ==================== SEARCH PLAYER ====================
void searchPlayer() {
    system("cls");
    cout << "\n========== SEARCH PLAYER ==========\n";
    cout << "Enter player name: ";
    string name;
    cin >> name;
    
    int score = playerHash.searchPlayer(name);
    if (score != -1) {
        cout << "\nPlayer found: " << name << " - " << score << " poin\n";
    } else {
        cout << "\nPlayer not found!\n";
    }
    
    cout << "\nPress any key to go back\n";
    getch();
}

// ==================== SHOW JALUR ====================
void showJalur() {
    system("cls");
    cout << "\n========== JALUR LEVEL ==========\n";
    
    Graph g(5);
    g.addEdge(1, 2);
    g.addEdge(1, 3);
    g.addEdge(2, 4);
    g.addEdge(3, 4);
    g.addEdge(4, 5);
    
    vector<int> path = g.BFS(1, 5);
    
    cout << "Jalur terpendek dari Start (1) ke End (5): ";
    for (int node : path) {
        cout << node << " ";
    }
    
    cout << "\n\nPress any key to go back\n";
    getch();
}
// ==================== FILE HANDLING ====================
void saveScoreToFile(string name, int score) {
    ofstream file(SCORE_FILE, ios::app);
    if (file.is_open()) {
        file << name << " " << score << endl;
        file.close();
    }
}

void loadScoresFromFile() {
    ifstream file(SCORE_FILE);
    if (!file.is_open()) return;

    string name;
    int score;

    while (file >> name >> score) {
        leaderboard.insertPlayer(name, score);
        playerHash.insertPlayer(name, score);
    }
    file.close();
}

// ==================== INSTRUCTIONS ====================
void instructions() {
    system("cls");
    cout << "Instructions\n";
    cout << "----------------\n";
    cout << "1. Avoid hurdles by jumping\n";
    cout << "2. Press 'Spacebar' to jump\n";
    cout << "3. Press 'p' to pause game\n";
    cout << "4. Press 'Escape' to exit game\n";
    cout << "5. Press 'r' to rollback score (Stack)\n";
    cout << "6. Press 'l' untuk lihat Leaderboard (BST+Sorting)\n";
    cout << "7. Press 's' untuk Search Player (Hashing)\n";
    cout << "8. Press 'j' untuk lihat Jalur (Graph+BFS)\n";
    cout << "\nPress any key to go back to menu\n";
    getch();
}
//=====================DELETE====================
void deleteScoreFromFile(string name) {
    ifstream fileIn(SCORE_FILE);
    if (!fileIn.is_open()) {
        cout << "Tidak ada file skor!\n";
        return;
    }

    vector<pair<string,int>> scores;
    string player;
    int score;

    // Baca semua skor
    while (fileIn >> player >> score) {
        if (player != name) { 
            scores.push_back({player, score}); // simpan skor selain yang mau dihapus
        }
    }
    fileIn.close();

    // Tulis ulang file tanpa skor yang dihapus
    ofstream fileOut(SCORE_FILE, ios::trunc);
    for (auto& p : scores) {
        fileOut << p.first << " " << p.second << endl;
    }
    fileOut.close();

    cout << "Skor untuk pemain " << name << " berhasil dihapus!\n";
}


// ==================== MAIN ====================
int main() {
    setcursor(0, 0);
	
	loadScoresFromFile();
    
    cout << "Enter your player name: ";
    cin >> playerName;
    
    do {
        system("cls");
        gotoxy(10, 5); cout << " -------------------------- ";
        gotoxy(10, 6); cout << " |        DINO RUN        | ";
        gotoxy(10, 7); cout << " -------------------------- ";
        gotoxy(10, 9); cout << "1. Start Game";
        gotoxy(10, 10); cout << "2. Instructions";
        gotoxy(10, 11); cout << "3. Leaderboard";
        gotoxy(10, 12); cout << "4. Search Player";
        gotoxy(10, 13); cout << "5. Show Jalur";
        gotoxy(10, 14); cout << "6. Delete Score";
		gotoxy(10, 15); cout << "7. Quit";
        gotoxy(10, 16); cout << "Select option: ";
        
        char op = getche();
        
        if (op == '1') play();
        else if (op == '2') instructions();
        else if (op == '3') showLeaderboard();
        else if (op == '4') searchPlayer();
        else if (op == '5') showJalur();
        else if (op == '6') {
			system("cls");
			cout << "Masukkan nama pemain yang ingin dihapus: ";
			string delName;
			cin >> delName;
			deleteScoreFromFile(delName);
			getch();
		}
		else if (op == '7') exit(0);

        
    } while (1);
    
    return 0;
	//653
}