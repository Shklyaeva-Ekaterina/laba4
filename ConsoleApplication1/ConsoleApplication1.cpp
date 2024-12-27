#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <set>
#include <vector>
#include <unordered_map>
#include <stack>
#include <queue>
#include <cmath>
#include <utility>
#include <limits>

using namespace std;

class CompressStation {
    friend void CreateCompress(map<int, CompressStation>& stations, int& idcounterCs);
    friend int ViewComp(map<int, CompressStation> stations, string answer);
    friend void EditCompress(CompressStation& C, int workshops);
    friend int MenuComp();
    friend void SearchComp(map<int, CompressStation>& stations);
    friend void Workshops(map<int, CompressStation>& stations);
    friend std::ostream& operator << (std::ostream& os, const CompressStation& C);
public:
    string name;
    int workshop;
    int realworkshop;
    double effect;
};

class Pipe {
    friend void CreatePipe(map<int, Pipe>& pipes, int& idcounterTr, int diametr);
    friend int ViewPipe(map<int, Pipe> pipes, string answer);
    friend void ReadFile(map<int, Pipe>& pipes, map<int, CompressStation>& stations);
    friend std::ostream& operator << (std::ostream& os, const Pipe& P);
public:
    string name;
    int length;
    int diametr;
    bool repair;
};

class PipeNet {
public:
    int from = -1; // ��������� �������
    int to = -1;   // �������� �������
    int weight = -1;
    int length = -1;

    void PN(int f, int t, int w,int l) {
        from = f, to = t, weight = w, length=l;
    }
    double flow() const {
        return std::sqrt(std::pow(weight, 5) / length);
    }
    void Edit(Pipe P) {
        if (!P.repair) {
            weight = P.diametr;
            length = P.length;
        }
        else{
            weight = 0;
            length = numeric_limits<int>::max();
        }
    }
};

class Net : private Pipe, private CompressStation {
    friend void Way(Net N);
    friend void EditPipe(Pipe& P, int id, map<int, Pipe> pipes, Net& N);
    friend void Flow(Net N);
    friend void NetAddition(int idcounterC, int& idcounterTr, map<int, Pipe>& pipes, map<int, CompressStation>& stations, Net& N);
private:
    unordered_map<int, CompressStation> netstations; // ������ �������
    unordered_map<int, PipeNet> netpipes;; // ������ �����

    // ��������������� ����� ��� DFS
    void topologicalSortUtil(int v, std::unordered_map<int, bool>& visited, std::stack<int>& Stack) {
        visited[v] = true; // �������� ������� ������� ��� ����������

        // ���������� �������� ��� ������� �������
        for (const auto& pipe : netpipes) {
            if (pipe.second.from == v) { // ���� ���� ����� �� v
                int neighbor = pipe.second.to;
                if (!visited[neighbor]) {
                    topologicalSortUtil(neighbor, visited, Stack);
                }
            }
        }

        // ��������� ������� ������� � ����
        Stack.push(v);
    }

public:
    // ����� ��� ���������� �������
    void addStation(map<int, CompressStation>& stations, int id) {
        netstations[id] = stations[id];
    }

    // ����� ��� ���������� �����
    void addPipe(map<int, Pipe>& pipes, int idin, int idout, int diametr, int& idcounterTr) {
        PipeNet netpipe;
        int control = 1;
        for (const auto& pair : pipes) {
            if (pair.second.diametr == diametr && !netpipes.count(pair.first)) {
                if (pair.second.repair) {
                    netpipe.PN(idin, idout, 0, numeric_limits<int>::max());
                }
                else {
                    netpipe.PN(idin, idout, pipes[pair.first].diametr, pipes[pair.first].length);
                }
                netpipes[pair.first] = netpipe;
                control = 0;
            }
            break;
        }
        if (control) {
            CreatePipe(pipes, idcounterTr, diametr);
            if (pipes[idcounterTr - 1].repair) {
                netpipe.PN(idin, idout, 0, numeric_limits<int>::max());
            }
            else {
                netpipe.PN(idin, idout, diametr, pipes[idcounterTr - 1].length);
            }
            netpipes[idcounterTr - 1] = netpipe;
        }
    }
    // ����� ��� ����������� �����
    void display() const {
        std::cout << "Stations:\n";
        for (const auto& pair : netstations) {
            std::cout << "ID: " << pair.first << "\n";
        }
        std::cout << "Pipes:\n";
        for (const auto& pair : netpipes) {
            std::cout << "ID: " << pair.first << "   From: " << pair.second.from << "   To: " << pair.second.to;
            if (pair.second.weight == 0) {
                cout << "   Under repair"<<endl;
            }
            else {
                cout << "   Weight: " << pair.second.weight << "   Length: " << pair.second.length << "\n";
            }
        }
    }

    // ����� ��� ���������� �������������� ����������
    void topologicalSort() {
        std::unordered_map<int, bool> visited; // ������ ��� ������������ ���������� ������
        std::stack<int> Stack; // ���� ��� �������� �������������� ����������

        // ����� ���� ������
        for (const auto& pair : netstations) {
            if (!visited[pair.first]) {
                topologicalSortUtil(pair.first, visited, Stack);
            }
        }

        // ����� ����������
        std::cout << "Topological Sort: ";
        while (!Stack.empty()) {
            std::cout << Stack.top() << " ";
            Stack.pop();
        }
        std::cout << std::endl;
    }

    // ����� ��� ������� ������������� ������
    double maxFlow(int source, int sink) {
        // ������� ���� ��� ������
        std::unordered_map<int, std::unordered_map<int, double>> capacity;
        for (const auto& pipe : netpipes) {
            capacity[pipe.second.from][pipe.second.to] += pipe.second.flow(); // ����������� �������
        }

        double totalFlow = 0;
        std::unordered_map<int, int> parent;

        while (true) {
            // BFS ��� ������ �������������� ����
            std::queue<int> q;
            q.push(source);
            parent.clear();
            parent[source] = -1;

            while (!q.empty()) {
                int current = q.front();
                q.pop();

                for (const auto& pair : capacity[current]) {
                    int neighbor = pair.first;
                    double cap = pair.second;

                    if (parent.find(neighbor) == parent.end() && cap > 0) { // ���� �� �������� � ���� �������
                        parent[neighbor] = current;
                        if (neighbor == sink) break; // ���� �������� �����
                        q.push(neighbor);
                    }
                }

                if (parent.find(sink) != parent.end()) break; // ���� ����� ����
            }

            // ���� �� ����� ����, �������
            if (parent.find(sink) == parent.end()) break;

            // ������� ����������� ������� � ��������� ����
            double pathFlow = std::numeric_limits<double>::max();
            for (int v = sink; v != source; v = parent[v]) {
                int u = parent[v];
                pathFlow = std::min(pathFlow, capacity[u][v]);
            }

            // ��������� �������
            for (int v = sink; v != source; v = parent[v]) {
                int u = parent[v];
                capacity[u][v] -= pathFlow;
                capacity[v][u] += pathFlow; // �������� �����
            }

            totalFlow += pathFlow; // ����������� ����� �����
        }

        return totalFlow;
    }

    // ����� ��� ������ ����������� ����
    std::vector<int> shortestPath(int source, int sink) {
        std::unordered_map<int, double> distances;
        std::unordered_map<int, int> previous;
        std::priority_queue<std::pair<double, int>, std::vector<std::pair<double, int>>, std::greater<>> pq;

        // ������������� ����������
        for (const auto& station : netstations) {
            distances[station.first] = std::numeric_limits<double>::infinity();
            previous[station.first] = -1; // ������������� ���������� �����
        }
        distances[source] = 0;
        pq.push({ 0, source });

        while (!pq.empty()) {
            int current = pq.top().second;
            pq.pop();

            // ���� �������� �����, ��������������� ����
            if (current == sink) {
                std::vector<int> path;
                for (int at = sink; at != -1; at = previous[at]) {
                    path.push_back(at);
                }
                std::reverse(path.begin(), path.end());
                return path;
            }

            // ����� �������
            for (const auto& pipe : netpipes) {
                if (pipe.second.from == current) {
                    int neighbor = pipe.second.to;
                    double weight = pipe.second.length;

                    // �������� �� ����������� �����
                    if (weight == std::numeric_limits<int>::max()) {
                        continue; // ���������� ����� � ����������� ������
                    }

                    // ������������ ����� ����������
                    double newDist = distances[current] + weight;
                    if (newDist < distances[neighbor]) {
                        distances[neighbor] = newDist;
                        previous[neighbor] = current;
                        pq.push({ newDist, neighbor });
                    }
                }
            }
        }

        return {}; // ���� ���� �� ������
    }
};


int MenuPipes();
int MenuComp();
void SaveFile(map<int, Pipe>& pipes, map<int, CompressStation>& stations);
void NetAddition(int idcounterC, int& idcounterTr, map<int, Pipe>& pipes, map<int, CompressStation>& stations, Net& N);
void Repair(bool filt, map<int, Pipe>& pipes, Net& N);
void SearchPipe(map<int, Pipe>& pipes, Net& N);

int Menu();
void Mistake();
int EditMenu();

std::ostream& operator << (std::ostream& os, const Pipe& P)
{
    string rep;
    if (P.repair) {
        rep = "Under repair";
    }
    else {
        rep = "Not under repair";
    }
    return cout << "\nName: " << P.name << "\nLength: " << P.length << "\nDiametr: " << P.diametr << endl << rep << endl;
}

std::ostream& operator << (std::ostream& os, const CompressStation& C)
{
    return cout << "\nName: " << C.name << "\nAmount of workshops: " << C.workshop << "\nAmount of working workshops: " << C.realworkshop << "\nEffectiveness: " << C.effect << "\n";
}


int main()
{
    Net N;
    map<int, Pipe> pipes;
    map<int, CompressStation> stations;
    int idcounterTr = 1;
    int idcounterCs = 1;
    bool cycle = 1;
    while (cycle) {
        int choice;
        choice = Menu();
        switch (choice) {
        case 1:
            CreatePipe(pipes, idcounterTr, 0);
            break;
        case 2:
            CreateCompress(stations, idcounterCs);
            break;
        case 3:
            ViewPipe(pipes, "any");
            break;
        case 4:
            ViewComp(stations, "any");
            break;
        case 5:
            bool filt;
            choice = MenuPipes();
            switch (choice) {
            case 1:
                filt = 1;
                Repair(filt, pipes, N);
                break;
            case 2:
                filt = 0;
                Repair(filt, pipes, N);
                break;
            case 3:
                SearchPipe(pipes, N);
                break;
            }
            break;
        case 6:
            choice = MenuComp();
            switch (choice) {
            case 1:
                SearchComp(stations);
                break;
            case 2:
                Workshops(stations);
                break;
            }
            break;
        case 7:
            SaveFile(pipes, stations);
            break;
        case 8:
            ReadFile(pipes, stations);
            break;
        case 9:
            if (stations.size() < 2) {
                cout << "You don't have enough stations. Create more." << endl;
            }
            else {
                NetAddition(idcounterCs, idcounterTr, pipes, stations, N);
            }
            break;
        case 10:
            N.display();
            break;
        case 11:
            N.topologicalSort();
            break;
        case 12:
            Flow(N);
            break;
        case 13:
            Way(N);
            break;
        case 0:
            cycle = 0;
            break;
        }
    }
};/////////////////////////////////////////////////////


//����� �������
int Menu() { // ������� ���� ��� ������ ��������
    int choice;
    cout << "\n...........Choose.an.option...........\n";
    cout << ".  1.Create a pipe                   .\n";
    cout << ".  2.Create a compressor station     .\n";
    cout << ".  3.View my pipes                   .\n";
    cout << ".  4.View my compressor stations     .\n";
    cout << ".  5.Search pipes                    .\n";
    cout << ".  6.Search compressor stations      .\n";
    cout << ".  7.Save                            .\n";
    cout << ".  8.Read                            .\n";
    cout << ".  9.Add to net                      .\n";
    cout << ".  10.View net                       .\n";
    cout << ".  11.Sort net                       .\n";
    cout << ".  12.Max flow                       .\n";
    cout << ".  13.Shortest way                   .\n";
    cout << ".  0.Exit                            .\n";
    cout << "......................................\n";
    while (1) {
        cin >> choice; // ��������� �����
        // ���������, ��������� �� ������� �����
        if (choice >= 0 && choice <= 13 && cin.good() && cin.peek() == '\n') {
            // ���� �� � �������, ������� �� �����
            break;
        }
        else {
            Mistake();//������� ��������� �� ������
        }
    }
    return choice;
};

void Mistake() {
    cout << "Incorrect input(((\nPlease try again:\n";
    cin.clear(); // ���������� ���� ������
    cin.ignore(numeric_limits<streamsize>::max(), '\n'); // ���������� ������������ ����
};

int EditMenu() {
    int choice;
    cout << "\n...........Choose.an.option...........\n";
    cout << ".  1.Edit                            .\n";
    cout << ".  2.Edit by choice                  .\n";
    cout << ".  2.Delete                          .\n";
    cout << ".  2.Delete by choice                .\n";
    cout << ".  0.Exit                            .\n";
    cout << "......................................\n";
    while (1) {
        cin >> choice; // ��������� �����
        // ���������, ��������� �� ������� �����
        if (choice >= 0 && choice <= 2 && cin.good() && cin.peek() == '\n') {
            // ���� �� � �������, ������� �� �����
            break;
        }
        else {
            Mistake();//������� ��������� �� ������
        }
    }
    return choice;
}


//������� ��� ������ � �������
void CreatePipe(map<int, Pipe>& pipes, int& idcounterTr, int diametr) {
    Pipe P;
    cout << "\nCreation of a pipe.\n";
    cout << "Name the pipe:\n";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    while (1) {
        getline(cin, P.name); // ��������� ���
        // ���������, ���� �� ������� ���
        if (cin.good()) {
            // ���� �� � �������, ������� �� ����� 
            break;
        }
        else {
            //cout << "You already have that pipe.\nTry another name:\n";//������� ��������� �� ������
            cin.clear(); // ���������� ���� ������
            continue;
        }
    }
    cout << "Specify the lenght:\n";
    while (1) {
        cin >> P.length; // ��������� �����
        // ���������, ��������� �� ������� �����
        if (P.length > 0 && cin.good() && cin.peek() == '\n') {
            // ���� �� � �������, ������� �� �����
            break;
        }
        else {
            Mistake();//������� ��������� �� ������
        }
    }
    if (diametr) {
        P.diametr = diametr;
    }
    else {
        cout << "Specify the diametr:\n";
        while (1) {
            cin >> P.diametr; // ��������� �����
            // ���������, ��������� �� ������� �����
            if (P.diametr > 0 && cin.good() && cin.peek() == '\n' && (P.diametr == 500 || P.diametr == 700 || P.diametr == 1000 || P.diametr == 1400)) {
                // ���� �� � �������, ������� �� �����
                break;
            }
            else {
                Mistake();//������� ��������� �� ������
            }
        }
    }
    cout << "Is it under repair?(y/n)\n";
    string answer;
    while (1) {
        cin >> answer; // ��������� �����
        // ���������, ��������� �� ������ �����
        if ((answer == "y" || answer == "n") && cin.good() && cin.peek() == '\n') {
            // ���� �� � �������, ������� �� �����
            break;
        }
        else {
            Mistake();//������� ��������� �� ������
        }
    }
    if (answer == "y") {
        P.repair = true;
    }
    else if (answer == "n") {
        P.repair = false;
    }
    pipes[idcounterTr] = P;
    idcounterTr++;
}

int ViewPipe(map<int, Pipe> pipes, string answer) {
    if (pipes.size() != 0) {
        cout << "\nPipes\n";
        for (const auto& pair : pipes) {
            cout << "\nID: " << pair.first;
            cout << pair.second;
        }
        return 1;
    }
    else {
        cout << "\nYou don't have " << answer << " pipes yet\n";
        return 0;
    }
}

int MenuPipes() {
    int choice;
    cout << "\n...........Choose.an.option...........\n";
    cout << ".  1.Search pipes under repair       .\n";
    cout << ".  2.Search pipes not under repair   .\n";
    cout << ".  3.Search by name                  .\n";
    cout << ".  0.Exit                            .\n";
    cout << "......................................\n";
    while (1) {
        cin >> choice; // ��������� �����
        // ���������, ��������� �� ������� �����
        if (choice >= 0 && choice <= 3 && cin.good() && cin.peek() == '\n') {
            // ���� �� � �������, ������� �� �����
            break;
        }
        else {
            Mistake();//������� ��������� �� ������
        }
    }
    return choice;
}

void Repair(bool filt, map<int, Pipe>& pipes,Net& N) {
    map<int, Pipe> repair;
    if (filt) {
        for (const auto& pair : pipes) {
            if (pair.second.repair) {
                repair[pair.first] = pair.second;
            }
        }
    }
    else {
        for (const auto& pair : pipes) {
            if (!pair.second.repair) {
                repair[pair.first] = pair.second;
            }
        }
    }
    int exist = ViewPipe(repair, "such");
    if (exist) {
        int choice = EditMenu();
        Pipe P;
        switch (choice) {
        case 1:
            for (const auto& pair : repair) {
                P = pair.second;
                pipes.erase(pair.first);
                EditPipe(P,pair.first,pipes,N);
                pipes[pair.first] = P;
            }
            break;
        case 2:
            cout << "Choose ids of pipes you want to edit (y/n):" << endl;
            for (const auto& pair : repair) {
                P = pair.second;
                pipes.erase(pair.first);
                cout << pair.first << endl;
                string answer;
                while (1) {
                    cin >> answer; // ��������� �����
                    // ���������, ��������� �� ������ �����
                    if (answer == "y" && cin.good() && cin.peek() == '\n') {
                        // ���� �� � �������, ������� �� �����
                        EditPipe(P, pair.first, pipes, N);
                        pipes[pair.first] = P;
                        break;
                    }
                    else if (answer == "n" && cin.good() && cin.peek() == '\n') {
                        pipes[pair.first] = P;
                        // ���� �� � �������, ������� �� �����
                        break;
                    }
                    else {
                        Mistake();//������� ��������� �� ������
                    }
                }
            }
            break;
        case 3:
            for (const auto& pair : repair) {
                pipes.erase(pair.first);
            }
            break;
        case 4:
            cout << "Choose ids of pipes you want to delete (y/n):" << endl;
            for (const auto& pair : repair) {
                P = pair.second;
                pipes.erase(pair.first);
                cout << pair.first << endl;
                string answer;
                while (1) {
                    cin >> answer; // ��������� �����
                    // ���������, ��������� �� ������ �����
                    if (answer == "y" && cin.good() && cin.peek() == '\n') {
                        // ���� �� � �������, ������� �� �����
                        break;
                    }
                    else if (answer == "n" && cin.good() && cin.peek() == '\n') {
                        pipes[pair.first] = P;
                        // ���� �� � �������, ������� �� �����
                        break;
                    }
                    else {
                        Mistake();//������� ��������� �� ������
                    }
                }
            }
            break;
        }
    }
}

void SearchPipe(map<int, Pipe>& pipes,Net& N) {
    string name;
    bool search = 0;
    map<int, Pipe> found;
    cout << "Name the pipe you want to find:\n";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    getline(cin, name); // ��������� ���
    for (const auto& pair : pipes) {
        if (pair.second.name.find(name) != string::npos) {
            found[pair.first] = pair.second;
            cout << "\nID: " << pair.first;
            cout << pair.second;
            search = 1;
        }
    }
    if (search) {
        int choice = EditMenu();
        Pipe P;
        switch (choice) {
        case 1:
            for (const auto& pair : found) {
                P = pair.second;
                pipes.erase(pair.first);
                EditPipe(P,pair.first,pipes,N);
                pipes[pair.first] = P;
            }
            break;
        case 2:
            cout << "Choose ids of pipes you want to edit (y/n):" << endl;
            for (const auto& pair : found) {
                P = pair.second;
                pipes.erase(pair.first);
                cout << pair.first << endl;
                string answer;
                while (1) {
                    cin >> answer; // ��������� �����
                    // ���������, ��������� �� ������ �����
                    if (answer == "y" && cin.good() && cin.peek() == '\n') {
                        // ���� �� � �������, ������� �� �����
                        EditPipe(P, pair.first, pipes, N);
                        pipes[pair.first] = P;
                        break;
                    }
                    else if (answer == "n" && cin.good() && cin.peek() == '\n') {
                        pipes[pair.first] = P;
                        // ���� �� � �������, ������� �� �����
                        break;
                    }
                    else {
                        Mistake();//������� ��������� �� ������
                    }
                }
            }
            break;
        case 3:
            for (const auto& pair : found) {
                pipes.erase(pair.first);
            }
            break;
        case 4:
            cout << "Choose ids of pipes you want to delete (y/n):" << endl;
            for (const auto& pair : found) {
                P = pair.second;
                pipes.erase(pair.first);
                cout << pair.first << endl;
                string answer;
                while (1) {
                    cin >> answer; // ��������� �����
                    // ���������, ��������� �� ������ �����
                    if (answer == "y" && cin.good() && cin.peek() == '\n') {
                        // ���� �� � �������, ������� �� �����
                        break;
                    }
                    else if (answer == "n" && cin.good() && cin.peek() == '\n') {
                        pipes[pair.first] = P;
                        // ���� �� � �������, ������� �� �����
                        break;
                    }
                    else {
                        Mistake();//������� ��������� �� ������
                    }
                }
            }
            break;
        }
    }
    else {
        cout << "You don't have that pipes\n";
    }
}

void EditPipe(Pipe& P,int id, map<int, Pipe> pipes, Net& N) {
    if (P.repair == 1) {
        P.repair = 0;
    }
    else {
        P.repair = 1;
    }
    PipeNet newpipe = N.netpipes[id];
    N.netpipes.erase(id);
    newpipe.Edit(P);
    N.netpipes[id] = newpipe;
}


//������� ��� ������ � ��
void CreateCompress(map<int, CompressStation>& stations, int& idcounterCs) {
    CompressStation C;
    cout << "\nCreation of a compressor station.\n";
    cout << "Name the compressor station:\n";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    while (1) {
        getline(cin, C.name); // ��������� ���
        // ���������, ���� �� ������� ���
        if (cin.good()) {
            // ���� �� � �������, ������� �� ����� 
            break;
        }
        else {
            //cout << "You already have that pipe.\nTry another name:\n";//������� ��������� �� ������
            cin.clear(); // ���������� ���� ������
            continue;
        }
    }
    cout << "Specify the amount of workshops:\n";
    while (1) {
        cin >> C.workshop; // ��������� �����
        // ���������, ��������� �� ������� �����
        if (C.workshop > 0 && cin.good() && cin.peek() == '\n') {
            // ���� �� � �������, ������� �� �����
            break;
        }
        else {
            Mistake();//������� ��������� �� ������
        }
    }
    cout << "Specify the amount of working workshops:\n";
    while (1) {
        cin >> C.realworkshop; // ��������� �����
        // ���������, ��������� �� ������� �����
        if (C.realworkshop >= 0 && C.realworkshop <= C.workshop && cin.good() && cin.peek() == '\n') {
            // ���� �� � �������, ������� �� �����
            break;
        }
        else {
            Mistake();//������� ��������� �� ������
        }
    }
    cout << "Specify the effectiveness:\n";
    while (1) {
        cin >> C.effect; // ��������� �����
        // ���������, ��������� �� ������� �����
        if (cin.good() && cin.peek() == '\n') {
            // ���� �� � �������, ������� �� �����
            break;
        }
        else {
            Mistake();//������� ��������� �� ������
        }
    }
    stations[idcounterCs] = C;
    idcounterCs++;

}

int ViewComp(map<int, CompressStation> stations, string answer) {
    if (stations.size() != 0) {
        cout << "\nCompressor Stations\n";
        for (const auto& pair : stations) {
            cout << "\nID: " << pair.first;
            cout << pair.second;
        }
        return 1;
    }
    else {
        cout << "\nYou don't have " << answer << " stations yet\n";
        return 0;
    }
}

void EditCompress(CompressStation& C, int workshops) {
    if (C.workshop >= workshops) {// ��������� �����
        C.realworkshop = workshops;
    }
    else {
        C.realworkshop = C.workshop;
    }
}

int MenuComp() {
    int choice;
    cout << "\n...............Choose.an.option...............\n";
    cout << ".  1.Search by name                          .\n";
    cout << ".  2.Search by percent of working workshops  .\n";
    cout << ".  0.Exit                                    .\n";
    cout << "..............................................\n";
    while (1) {
        cin >> choice; // ��������� �����
        // ���������, ��������� �� ������� �����
        if (choice >= 0 && choice <= 2 && cin.good() && cin.peek() == '\n') {
            // ���� �� � �������, ������� �� �����
            break;
        }
        else {
            Mistake();//������� ��������� �� ������
        }
    }
    return choice;
}

void SearchComp(map<int, CompressStation>& stations) {
    string name;
    bool search = 0;
    map<int, CompressStation> found;
    cout << "Name the pipe you want to find:\n";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    getline(cin, name); // ��������� ���
    for (const auto& pair : stations) {
        if (pair.second.name.find(name) != string::npos) {
            found[pair.first] = pair.second;
            cout << "\nID: " << pair.first;
            cout << pair.second;
            search = 1;
        }
    }
    if (search) {
        int workshops;
        int choice = EditMenu();
        CompressStation C;
        switch (choice) {
        case 1:
            cout << "Specify the amount of working workshops:\n";
            while (1) {
                cin >> workshops;
                if (cin.good() && cin.peek() == '\n') {
                    // ���� �� � �������, ������� �� �����
                    break;
                }
                else {
                    Mistake();//������� ��������� �� ������
                }
            }
            for (const auto& pair : found) {
                C = pair.second;
                stations.erase(pair.first);
                EditCompress(C, workshops);
                stations[pair.first] = C;
            }
            break;
        case 2:
            cout << "Specify the amount of working workshops:\n";
            while (1) {
                cin >> workshops;
                if (cin.good() && cin.peek() == '\n') {
                    // ���� �� � �������, ������� �� �����
                    break;
                }
                else {
                    Mistake();//������� ��������� �� ������
                }
            }
            cout << "Choose ids of stations you want to edit (y/n):" << endl;
            for (const auto& pair : found) {
                C = pair.second;
                stations.erase(pair.first);
                cout << pair.first << endl;
                string answer;
                while (1) {
                    cin >> answer; // ��������� �����
                    // ���������, ��������� �� ������ �����
                    if (answer == "y" && cin.good() && cin.peek() == '\n') {
                        // ���� �� � �������, ������� �� �����
                        EditCompress(C, workshops);
                        stations[pair.first] = C;
                        break;
                    }
                    else if (answer == "n" && cin.good() && cin.peek() == '\n') {
                        stations[pair.first] = C;
                        // ���� �� � �������, ������� �� �����
                        break;
                    }
                    else {
                        Mistake();//������� ��������� �� ������
                    }
                }
            }
            break;
        case 3:
            for (const auto& pair : found) {
                stations.erase(pair.first);
            }
            break;
        case 4:
            cout << "Choose ids of stations you want to delete (y/n):" << endl;
            for (const auto& pair : found) {
                C = pair.second;
                stations.erase(pair.first);
                cout << pair.first << endl;
                string answer;
                while (1) {
                    cin >> answer; // ��������� �����
                    // ���������, ��������� �� ������ �����
                    if (answer == "y" && cin.good() && cin.peek() == '\n') {
                        // ���� �� � �������, ������� �� �����
                        break;
                    }
                    else if (answer == "n" && cin.good() && cin.peek() == '\n') {
                        stations[pair.first] = C;
                        // ���� �� � �������, ������� �� �����
                        break;
                    }
                    else {
                        Mistake();//������� ��������� �� ������
                    }
                }
            }
            break;
        }
    }
    else {
        cout << "You don't have that compressor station\n";
    }
}

void Workshops(map<int, CompressStation>& stations) {
    map<int, CompressStation> percents;
    int percent;

    cout << "Specify the percent of working workshops:\n";
    while (1) {
        cin >> percent; // ��������� �����
        // ���������, ��������� �� ������� �����
        if (percent > 0 && cin.good() && cin.peek() == '\n') {
            // ���� �� � �������, ������� �� �����
            break;
        }
        else {
            Mistake();//������� ��������� �� ������
        }
    }
    bool search = 0;
    for (const auto& pair : stations) {
        if (((abs(1.0 * pair.second.workshop - 1.0 * pair.second.realworkshop) / pair.second.workshop) * 100.0 - 1.0 * percent) < 10.0) {
            percents[pair.first] = pair.second;
            cout << "\nID: " << pair.first;
            cout << pair.second;
            search = 1;
        }
    }
    if (search) {
        int workshops;
        int choice = EditMenu();
        CompressStation C;
        switch (choice) {
        case 1:
            cout << "Specify the amount of working workshops:\n";
            while (1) {
                cin >> workshops;
                if (cin.good() && cin.peek() == '\n') {
                    // ���� �� � �������, ������� �� �����
                    break;
                }
                else {
                    Mistake();//������� ��������� �� ������
                }
            }
            for (const auto& pair : percents) {
                C = pair.second;
                stations.erase(pair.first);
                EditCompress(C, workshops);
                stations[pair.first] = C;
            }
            break;
        case 2:
            cout << "Specify the amount of working workshops:\n";
            while (1) {
                cin >> workshops;
                if (cin.good() && cin.peek() == '\n') {
                    // ���� �� � �������, ������� �� �����
                    break;
                }
                else {
                    Mistake();//������� ��������� �� ������
                }
            }
            cout << "Choose ids of stations you want to edit (y/n):" << endl;
            for (const auto& pair : percents) {
                C = pair.second;
                stations.erase(pair.first);
                cout << pair.first << endl;
                string answer;
                while (1) {
                    cin >> answer; // ��������� �����
                    // ���������, ��������� �� ������ �����
                    if (answer == "y" && cin.good() && cin.peek() == '\n') {
                        // ���� �� � �������, ������� �� �����
                        EditCompress(C, workshops);
                        stations[pair.first] = C;
                        break;
                    }
                    else if (answer == "n" && cin.good() && cin.peek() == '\n') {
                        stations[pair.first] = C;
                        // ���� �� � �������, ������� �� �����
                        break;
                    }
                    else {
                        Mistake();//������� ��������� �� ������
                    }
                }
            }
            break;
        case 3:
            for (const auto& pair : percents) {
                stations.erase(pair.first);
            }
            break;
        case 4:
            cout << "Choose ids of stations you want to delete (y/n):" << endl;
            for (const auto& pair : percents) {
                C = pair.second;
                stations.erase(pair.first);
                cout << pair.first << endl;
                string answer;
                while (1) {
                    cin >> answer; // ��������� �����
                    // ���������, ��������� �� ������ �����
                    if (answer == "y" && cin.good() && cin.peek() == '\n') {
                        // ���� �� � �������, ������� �� �����
                        break;
                    }
                    else if (answer == "n" && cin.good() && cin.peek() == '\n') {
                        stations[pair.first] = C;
                        // ���� �� � �������, ������� �� �����
                        break;
                    }
                    else {
                        Mistake();//������� ��������� �� ������
                    }
                }
            }
            break;
        }
    }
    else {
        cout << "You don't have that compressor station\n";
    }
}

void SaveFile(map<int, Pipe>& pipes, map<int, CompressStation>& stations) {
    string filename;
    cout << "Choose the file to save information:" << endl;
    while (1) {
        cin >> filename;
        // ���������, ��������� �� ������� �����
        if (filename.length() > 4 && cin.good() && cin.peek() == '\n' && filename.compare(filename.length() - 4, 4, ".txt") == 0) {
            // ���� �� � �������, ������� �� �����
            break;
        }
        else {
            Mistake();//������� ��������� �� ������
        }
    }

    // ������� ������ ofstream ��� ������ � ����
    ofstream outFile(filename);
    for (const auto& pair : pipes) {
        outFile << "p\n";
        outFile << pair.first;
        outFile << endl << pair.second.name << endl << pair.second.length << endl << pair.second.diametr << endl << pair.second.repair << endl;
    }
    for (const auto& pair : stations) {
        outFile << "s\n";
        outFile << pair.first;
        outFile << endl << pair.second.name << endl << pair.second.workshop << endl << pair.second.realworkshop << endl << pair.second.effect << endl;
    }
    // ��������� ����
    outFile.close();
}

void ReadFile(map<int, Pipe>& pipes, map<int, CompressStation>& stations) {
    string filename;
    cout << "Choose the file to read information:" << endl;
    while (1) {
        cin >> filename;
        // ���������, ��������� �� ������� �����
        if (filename.length() > 4 && cin.good() && cin.peek() == '\n' && filename.compare(filename.length() - 4, 4, ".txt") == 0) {
            // ���� �� � �������, ������� �� �����
            break;
        }
        else {
            Mistake();//������� ��������� �� ������
        }
    }
    ifstream inFile(filename);
    // ���������, ������� �� ������� ����
    if (inFile) {
        string line;
        // ������ ����� �� �����
        string fromfile;
        int lineNumber = 0;
        int mark = 0;
        int id;
        while (getline(inFile, line)) {
            cout << line << endl;
            inFile >> fromfile;
            if (fromfile == "p") {
                lineNumber = 0;
                mark = 1;
            }
            if (fromfile == "s") {
                lineNumber = 0;
                mark = 2;
            }
            if (mark == 2) {
                CompressStation C;
                if (lineNumber == 1) { // ���� ��� ������ ������
                    // ��������� �������� �����
                    id = stoi(fromfile);
                }
                if (lineNumber == 2) { // ���� ��� ������ ������
                    // ��������� �������� �����
                    C.name = fromfile;
                }
                if (lineNumber == 3) {
                    C.workshop = stoi(fromfile);
                }
                if (lineNumber == 4) {
                    C.realworkshop = stoi(fromfile);
                }
                if (lineNumber == 5) {
                    C.effect = stod(fromfile);
                    cout << C;
                    stations[id] = C;
                }
            }
            else if (mark == 1) {
                Pipe P;
                if (lineNumber == 1) { // ���� ��� ������ ������
                    // ��������� �������� �����
                    id = stoi(fromfile);
                }
                if (lineNumber == 2) { // ���� ��� ������ ������
                    // ��������� �������� �����
                    P.name = fromfile;
                }
                if (lineNumber == 3) {
                    // ��������� �������� �����

                    P.length = stoi(fromfile);
                }
                if (lineNumber == 4) {
                    P.diametr = stoi(fromfile);
                }
                if (lineNumber == 5) {
                    if (fromfile == "1") {
                        P.repair = 1;
                    }
                    else {
                        P.repair = 0;
                    }
                    cout << P;

                    pipes[id] = P;
                }
            }
            lineNumber++;
        }
    }
    // ��������� ����
    inFile.close();
}


void Flow(Net N) {
    if (N.netstations.size() > 0) {
        cout << "\nMax total flow.\n";
        cout << "Specify the in id:\n";
        int idin;
        while (1) {
            cin >> idin; // ��������� �����
            // ���������, ��������� �� ������� �����
            if (idin > 0 && cin.good() && cin.peek() == '\n' && N.netstations.count(idin)) {
                // ���� �� � �������, ������� �� �����
                break;
            }
            else {
                Mistake();//������� ��������� �� ������
            }
        }
        cout << "Specify the out id:\n";
        int idout;
        while (1) {
            cin >> idout; // ��������� �����
            // ���������, ��������� �� ������� �����
            if (idout > 0 && cin.good() && cin.peek() == '\n' && N.netstations.count(idout)) {
                // ���� �� � �������, ������� �� �����
                break;
            }
            else {
                Mistake();//������� ��������� �� ������
            }
        }
        double flow = N.maxFlow(idin, idout); // ����� �� Station A �� Station C
        cout << "Max flow from station " << idin << " to station " << idout << " is " << flow << endl;
    }
    else {
        cout << "Your net is empty. Add at least one station." << endl;
    }
}
void NetAddition(int idcounterC, int& idcounterTr, map<int, Pipe>& pipes, map<int, CompressStation>& stations, Net& N) {
    cout << "\nCreation of a net.\n";
    cout << "Specify the in id:\n";
    int idin;
    while (1) {
        cin >> idin; // ��������� �����
        // ���������, ��������� �� ������� �����
        if (idin > 0 && cin.good() && cin.peek() == '\n' && idin < idcounterC) {
            // ���� �� � �������, ������� �� �����
            break;
        }
        else {
            Mistake();//������� ��������� �� ������
        }
    }
    cout << "Specify the out id:\n";
    int idout;
    while (1) {
        cin >> idout; // ��������� �����
        // ���������, ��������� �� ������� �����
        if (idout > 0 && cin.good() && cin.peek() == '\n' && idout < idcounterC && idout != idin) {
            // ���� �� � �������, ������� �� �����
            break;
        }
        else {
            Mistake();//������� ��������� �� ������
        }
    }
    cout << "Specify the diametr:\n";
    int diametr;
    while (1) {
        cin >> diametr; // ��������� �����
        // ���������, ��������� �� ������� �����
        if (diametr > 0 && cin.good() && cin.peek() == '\n' && (diametr == 500 || diametr == 700 || diametr == 1000 || diametr == 1400)) {
            // ���� �� � �������, ������� �� �����
            break;
        }
        else {
            Mistake();//������� ��������� �� ������
        }
    }
    N.addPipe(pipes, idin, idout, diametr, idcounterTr);
    if (N.netstations.count(idin) <= 0) {
        N.addStation(stations, idin);
    }
    if (N.netstations.count(idout) <= 0) {
        N.addStation(stations, idout);
    }
}
void Way(Net N) {
    if (N.netstations.size() > 0) {
        cout << "\nShortest way.\n";
        cout << "Specify the in id:\n";
        int idin;
        while (1) {
            cin >> idin; // ��������� �����
            // ���������, ��������� �� ������� �����
            if (idin > 0 && cin.good() && cin.peek() == '\n' && N.netstations.count(idin)) {
                // ���� �� � �������, ������� �� �����
                break;
            }
            else {
                Mistake();//������� ��������� �� ������
            }
        }
        cout << "Specify the out id:\n";
        int idout;
        while (1) {
            cin >> idout; // ��������� �����
            // ���������, ��������� �� ������� �����
            if (idout > 0 && cin.good() && cin.peek() == '\n' && N.netstations.count(idout)) {
                // ���� �� � �������, ������� �� �����
                break;
            }
            else {
                Mistake();//������� ��������� �� ������
            }
        }
        std::vector<int> path = N.shortestPath(idin, idout); // ���������� ���� �� Station A �� Station C
        std::cout << "Shortest way from "<<idin<<" to "<<idout << " statation:"<<endl;
        for (int id : path) {
            std::cout << id << " ";
        }
        std::cout << std::endl;
    }
    else {
        cout << "Your net is empty. Add at least one station." << endl;
    }
}