#include "board.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <cctype>
#include <vector>
#include <string>
#include <sstream>

void Board::saveGame() {
    std::string filename = "savegame.txt";
    std::ofstream file(filename);
    if (!file.is_open()) {
        throw std::invalid_argument("Error opening file " + filename);
        return;
    }

    for (const auto &player : players) {
        file << player->getName() << ","
             << player->getPiece() << ","
             << player->getTimsCups() << ","
             << player->getWallet() << ","
             << player->getPosition() << ","
             << player->isVisitingTims() << "," 
             << player->getTimsLine() << std::endl;
    }

    for (const auto &tile : buildings) {
        if (auto ab = std::dynamic_pointer_cast<AcademicBuilding>(tile)) {
            if (ab->isOwned()) {
                file << "AcademicBuilding: " << ab->getName() << "," << bank->getPropertyOwner(ab->getName()) << "," << ab->isMortgaged() << "," << ab->getImpCount() << "," << ab->getImpCost() << std::endl;
            }
            else {
                file << "AcademicBuilding: " << ab->getName() << "," << "false" << "," << ab->isMortgaged() << "," << ab->getImpCount() << "," << ab->getImpCost() << std::endl;
            }
        }
        else if (auto r = std::dynamic_pointer_cast<Residence>(tile)) {
            if (r->isOwned()){
                file << "Residence: " << r->getName() << "," << bank->getPropertyOwner(r->getName()) << "," << r->isMortgaged() << std::endl;
            }
            else {
                file << "Residence: " << r->getName() << "," << "false" << "," << r->isMortgaged() << std::endl;
            }
            
        }
        else if (auto g = std::dynamic_pointer_cast<Gym>(tile)) {
            if (g->isOwned()) {
                file << "Gym: " << g->getName() << "," << bank->getPropertyOwner(g->getName()) << "," << g->isMortgaged() << std::endl;
            }
            else {
                file << "Gym: " << g->getName() << "," << "false" << "," << g->isMortgaged() << std::endl;
            }
        }
        else {
            throw std::invalid_argument("Unknown tile type.");
            return;
        }
        
    }

    file << "Current turn:" << playerTurn << std::endl;
    file.close();
    std::cout << "Game saved to " << filename << std::endl;
}


void Board::loadGame(const std::string &filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::invalid_argument("Error opening file " + filename);
        return;
    }

    players.clear();
    buildings.clear();
    playerTurn = 0;

    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string token;
        std::getline(ss, token, ':');
        if (token == "Player") {
            std::string name, piece, walletStr, positionStr, timsLineStr, timsCupsStr, visitingTimsStr;
            std::getline(ss, name, ',');
            std::getline(ss, piece, ',');
            std::getline(ss, walletStr, ',');
            std::getline(ss, positionStr, ',');
            std::getline(ss, visitingTimsStr, ',');
            std::getline(ss, timsLineStr, ',');
            std::getline(ss, timsCupsStr, ',');

            // Convert strings to integers
            int wallet = std::stoi(walletStr);
            int position = std::stoi(positionStr);
            bool visitingTims = (visitingTimsStr == "false");
            int timsLine = std::stoi(timsLineStr);
            int timsCups = std::stoi(timsCupsStr);
            const int boardSize = 40;
            auto player = std::make_shared<Player>(piece[0], name, wallet, boardSize, *bank, boardSize, position, visitingTims, timsLine, timsCups);
            players.push_back(player);
        }
        else if (token == "AcademicBuilding") {
            std::string name, ownerStr, isMortgagedStr;
            int impCount, impCost;
            bool isMortgaged, isOwned;
            std::getline(ss, name, ',');
            std::getline(ss, ownerStr, ',');
            std::getline(ss, isMortgagedStr, ',');
            std::string impCountStr, impCostStr;
            std::getline(ss, impCountStr, ',');
            std::getline(ss, impCostStr, ',');

            // Convert strings to integers
            impCount = std::stoi(impCountStr);
            impCost = std::stoi(impCostStr);

            // Convert strings to boolean
            isMortgaged = (isMortgagedStr == "true");
            isOwned = (ownerStr != "false");
            auto academicBuilding = std::make_shared<AcademicBuilding>(name, isOwned, isMortgaged, impCount, impCost);
            if (isOwned) {
                bank->addPropertyOwner(name, ownerStr);
            }
            buildings.push_back(academicBuilding);
        }
        else if (token == "Residence") {
            std::string name, ownerStr, isMortgagedStr;
            bool isMortgaged, isOwned;
            std::getline(ss, name, ',');
            std::getline(ss, ownerStr, ',');
            std::getline(ss, isMortgagedStr, ',');

            isMortgaged = (isMortgagedStr == "true");
            isOwned = (ownerStr != "false");
            auto residence = std::make_shared<Residence>(name, isOwned, isMortgaged);
            buildings.push_back(residence);
        }
        else if (token == "Gym") {
            std::string name, ownerStr, isMortgagedStr;
            bool isMortgaged, isOwned;
            std::getline(ss, name, ',');
            std::getline(ss, ownerStr, ',');
            std::getline(ss, isMortgagedStr, ',');

            isMortgaged = (isMortgagedStr == "true");
            isOwned = (ownerStr != "false");
            auto gym = std::make_shared<Gym>(name, isOwned, isMortgaged);
            buildings.push_back(gym);
        }
        else if (token == "Current turn") {
            ss >> playerTurn;
            ss.ignore();
        }
        else {
            throw std::invalid_argument("Unknown token: " + token);
            return;
        }
    }
    file.close();
}


int Board::getTurn() {
    return playerTurn;
}

void removeAllWhitespace(std::string &str)
{
    str.erase(std::remove_if(str.begin(), str.end(), [](unsigned char c)
                             { return std::isspace(c); }),
              str.end());
}

void Board::setupGame(const std::string &TileOrder) {
    
    std::ifstream file(TileOrder);
    if (!file.is_open()) {
        throw std::invalid_argument("Error opening file " + TileOrder);
        return;
    }
    std::string line;
    std::getline(file,line);
    int count = 0;
    while (getline(file, line)) {
        
        std::string buildingType, buildingName;
        std::istringstream iss(line);
        std::getline(iss, buildingType, ',');
        removeAllWhitespace(buildingType);
        std::getline(iss, buildingName);
        removeAllWhitespace(buildingName);

        if (buildingType == "AB") {
            std::shared_ptr<AcademicBuilding> tile = std::make_shared<AcademicBuilding>(buildingName, count, false, false, 0, 0);
            buildings.push_back(tile);
        }
        else if (buildingType == "R") {
            std::shared_ptr<Residence> tile = std::make_shared<Residence>(buildingName, count, false, false);
            buildings.push_back(tile);
        }
        else if (buildingType == "GYM") {
            std::shared_ptr<Gym> tile = std::make_shared<Gym>(buildingName, count, false, false);
            buildings.push_back(tile);
        }
        else if (buildingType == "NOP") {
            if (buildingName == "CollectOsap") {
                std::shared_ptr<CollectOsap> tile = std::make_shared<CollectOsap>(buildingName, count);
                buildings.push_back(tile);
            }
            else if (buildingName == "DCTims") {
                std::shared_ptr<DCTims> tile = std::make_shared<DCTims>(buildingName, count);
                buildings.push_back(tile);
            }
            else if (buildingName == "GoToTims") {
                std::shared_ptr<GoToTims> tile = std::make_shared<GoToTims>(buildingName, count);
                buildings.push_back(tile);
            }
            else if (buildingName == "GooseNesting") {
                std::shared_ptr<GooseNesting> tile = std::make_shared<GooseNesting>(buildingName, count);
                buildings.push_back(tile);
            }
            else if (buildingName == "Tuition") {
                std::shared_ptr<Tuition> tile = std::make_shared<Tuition>(buildingName, count);
                buildings.push_back(tile);
            }
            else if (buildingName == "CoopFee") {
                std::shared_ptr<CoopFee> tile = std::make_shared<CoopFee>(buildingName, count);
                buildings.push_back(tile);
            }
            else if (buildingName == "SLC") {
                std::shared_ptr<SLC> tile = std::make_shared<SLC>(buildingName, count);
                buildings.push_back(tile);
            }
            else if (buildingName == "NH") {
                std::shared_ptr<NH> tile = std::make_shared<NH>(buildingName, count);
                buildings.push_back(tile);
            }
            
        }
        else {
            throw std::invalid_argument("Unknown building type: " + buildingType);
            return;
        }
        ++count;
    }
    file.close();
}

std::shared_ptr<Player> Board::setPlayer(std::map<std::string, char> &nameToPiece) {
    std::string name;
    while(true) {
        std::cout << "Enter player's name: " << std::endl;
        std::getline(std::cin, name);

        std::string check = name;
        std::transform(check.begin(), check.end(), check.begin(), ::tolower);
        if (check == "bank") {
            throw std::invalid_argument("Invalid name. Player not added.");
            continue;
        }

        if (name.empty()) {
            throw std::invalid_argument("Invalid name. Player not added.");
            continue;
        }

        if (nameToPiece.find(name) != nameToPiece.end()) {
            throw std::invalid_argument("Duplicate name. Player not added.");
            continue;
        }
    }

    char playerPiece;
    while (true) {
        std::cout << "Enter player's piece out of:\n"
                  << "G (Goose)\n"
                  << "B (GRTBus)\n"
                  << "D (TimHortonsDoughnut)\n"
                  << "P (Professor)\n"
                  << "S (Student)\n"
                  << "$ (Money)\n"
                  << "L (Laptop)\n"
                  << "T (PinkTie)" << std::endl;
        std::cin >> playerPiece;
        switch(playerPiece) {
        case 'G':
        case 'B':
        case 'D':
        case 'P':
        case 'S':
        case '$':
        case 'L':
        case 'T':

            bool chosen = false;
            for (const auto& pair : nameToPiece) {
                if (pair.second == playerPiece) {
                    chosen = true; // Piece already chosen
                }
            }
            if (chosen) {
                throw std::invalid_argument("This piece has already been taken. Please try again.");
                continue;
            }
            std::cout << "You chose the " << playerPiece << " piece." << std::endl;
            break;
        default:
            throw std::invalid_argument("Invalid choice. Please try again.");
            continue;
            
        }
        break;
    };
    const int wallet = 1500;
    const int boardSize = 40;
    std::shared_ptr<Player> player = std::make_shared<Player>(playerPiece, name, wallet, *bank, boardSize);
    return player;
}


void Board::playGame() {
    std::cout << "Enter the number of players: " << std::endl;
    int numPlayers;
    std::cin >> numPlayers;
    std::cin.ignore();

    std::map<std::string, char> nameToPiece;

    for (int i = 0; i < numPlayers; ++i)
    {
        std::shared_ptr<Player> player = setPlayer(nameToPiece);
        nameToPiece[player->getName()] = player->getPiece();
        players.push_back(player);
    }

    if (numPlayers == 1)
    {
        std::shared_ptr<Player> player = players[0];
        std::string name = player->getName();
        std::cout << "Congratulations! " << name << " has won the game" << std::endl;
    }
    playerTurn = 0;

    std::vector<std::shared_ptr<OwnableProperty>> props;
    for (const auto &tile : buildings)
    {
        if (std::shared_ptr<OwnableProperty> ownable = std::dynamic_pointer_cast<OwnableProperty>(tile))
        {
            props.push_back(ownable);
        }
    }
    std::cout << "Game started with " << numPlayers << " players." << std::endl;
    bank->initBank(players, props);

    if (players.size() < 1) { std::cout << "Game over" << std::endl; }

    std::string cmd, name, prop1, prop2; // initial command : player name that may follow : 1st property name that may follow : 2nd property name that may follow
    std::vector<std::string> commands = {"roll : player rolls the dice twice and moves the sum of the two dice"
                                        ,"next : gives control to the next player"
                                        ,"trade <name> <give> <recieve> : offers a trade to <name> with the current player offering <give> and requesting <receive>"
                                        ,"improve <property> buy/sell : attemps to buy or sell an improvement for <property>"
                                        ,"mortgage <property> : attempts to mortgage <property>"
                                        ,"unmortgage <property> : attempts to unmortgage <property>"
                                        ,"bankrupt : current player declares bankrupcy"
                                        ,"assets : displays the assets of the current player"
                                        ,"all : displays the assests of every player"
                                        ,"save <filename> : saves the current game into a save file"
                                        ,"DCcup : uses a Roll up the Rim cup to get out of the DCTims line (must be in the DCTims line)"
                                        ,"help : lists all the available commands" };

    bool hasRolled = false; // Bool to track whether the current player has rolled or not
    int doubCount = 0; // Counts the number of doubles the current player has rolled
    std::shared_ptr<Player> curPlayer = players[playerTurn]; // Current player
    while (true) {
        
        std::cout << "It is currently " << curPlayer->getName() << "'s turn" << std::endl;
        std::cin >> cmd;

        if (cmd == "roll") {
            // Player rolls the dice and moves
            if (hasRolled) {
                std::cout << "You have already rolled, you can't roll again" << std::endl;
                continue;
            }

            // Blocks player from rolling if they have to pay a fee (this is if player rolls doubles and can roll again)
            if (curPlayer->hasToPay()) {
                std::cout << "You must pay a fee of $" << curPlayer->getFee() << " before ending your turn" << std::endl;
                continue;
            }

            int d1 = curPlayer->roll();
            int d2 = curPlayer->roll();
            int sum = d1 + d2;

            std::cout << "You rolled a " << d1 << " and a " << d2 << '!' << std::endl;
            if (d1 == d2) { // rolled doubles
                if (curPlayer->isVisitingTims())  {
                    std::cout << "You rolled doubles and successfully escaped the DCTims line!" << std::endl;
                    std::cout << "Move " << sum << " squares" << std::endl;
                    curPlayer->toggleVisiting();
                } else {
                    doubCount++;
                    if (doubCount == 3) {
                        std::cout << "You rolled so many doubles you decide to take a break in the DCTims line" << std::endl;
                        curPlayer->setPosition(10);
                        curPlayer->toggleVisiting();
                        doubCount = 0;
                        hasRolled = true;
                    }

                    std::cout << "You rolled doubles! You get to roll again!" << std::endl;
                    std::cout << "But be careful, if you roll 3 doubles in a row you go directly to the DCTims line! ";
                    std::cout << "Current number of doubles rolled: " << doubCount << std::endl;
                }
            } else {
                hasRolled = true;
                doubCount = 0;
            }

            if (!curPlayer->isVisitingTims()) {
                std::cout << "You are in the DCTims line so you won't move" << std::endl;
                curPlayer->movePosition(0);
            } else {
                std::cout << "You move " << sum << " squares" << std::endl;
                curPlayer->movePosition(sum);
            }
            

        } else if (cmd == "next") {
            // Moves to the next player's turn. Requires the current player to roll before calling
            if (curPlayer->hasToPay()) { // Blocks player from ending turn if they have to pay a fee
                std::cout << "You must pay a fee of $" << curPlayer->getFee() << " before ending your turn" << std::endl;
                continue;
            }

            // Ends current player's turn, moves to the next player
            doubCount = 0;
            hasRolled = false;
            this->nextTurn();
            curPlayer = players[playerTurn]; // Updates current player

        } else if (cmd == "pay") {
            // If player is in Jail or landed on a tile which requires them to pay a fee then they call this to pay
            // next can't be called if they need to pay a fee
            if (curPlayer->isVisitingTims() && !curPlayer->hasToPay()) {
                std::cout << "There are no fees for you to pay" << std::endl;
                continue;
            }

            if (!curPlayer->isVisitingTims()) { // Paying Jail Fee
                if (bank->transferFunds(curPlayer->getName(), "BANK", 50)) {
                    std::cout << "Successfully paid DCTims fee" << std::endl;
                    std::cout << "You have left the DCTims line" << std::endl;
                    curPlayer->toggleVisiting();
                    if (curPlayer->hasToPay()) curPlayer->toggleHasToPay();
                } else {
                    std::cout << "Failed to pay fee: mortgage properties or sell improvements to raise enough cash to pay the fee" << std::endl;
                    std::cout << "You currently have: $" << curPlayer->getWallet() << " and need to pay $50 to get out of the DCTims line" << std::endl;
                }
            }

            if (curPlayer->hasToPay()) { // Paying property/tuition/NH fee
                if (bank->transferFunds(curPlayer->getName(), curPlayer->getFeeOwner(), curPlayer->getFee())) {
                    std::cout << "Successfully paid fee" << std::endl;
                    curPlayer->toggleHasToPay();
                } else {
                    std::cout << "Failed to pay fee: mortgage properties or sell improvements to raise enough cash to pay the fee" << std::endl;
                    std::cout << "You currently have: $" << curPlayer->getWallet() << " and need to pay $" << curPlayer->getFee() << std::endl;
                }
            }

        } else if (cmd == "DCcup") {
            // Can only be called if player is in Jail, uses a tims cup if they have it
            if (curPlayer->isVisitingTims()) {
                std::cout << "You are not currently in the DCTims line" << std::endl;
                continue;
            }

            if (curPlayer->getTimsCups() > 0) {
                curPlayer->addTimsCups(-1);
                curPlayer->toggleVisiting();
                bank->addDCTimsCups(-1);
                std::cout << "You are out of the DCTims line" << std::endl;
            }

        } else if (cmd == "buy" ) {
            // Can only be called if player is on an unowned property
            // The property will go up for auction if "next" is entered without the player buying
            if (!curPlayer->canBuy()) { // Player has the option of buying an unowned property
                std::cout << "You can't buy the property you are currently on" << std::endl;
                continue;
            }

            int pcost = this->getPropCost(curPlayer->getPosition()); // cost of property
            std::string pname = this->getTileName(curPlayer->getPosition()); // name of property

            if (bank->transferFunds(curPlayer->getName(), "BANK", pcost)) {
                std::cout << "You have successfully bought " << pname << std::endl;
                bank->transferProperty(curPlayer->getName(), pname);
            } else {
                std::cout << "You have insufficient funds to buy this property" << std::endl;
                std::cout << "Cash in your wallet: $" << curPlayer->getWallet() << std::endl;
                std::cout << "Cost of " << pname << ": $" << pcost << std::endl;
            }

        } else if (cmd == "trade") {
            // Initiates trade with another player
            std::cin >> name;
            
            if (!playerExists(name)) {
                std::cout << "Player " << name << " does not exist" << std::endl;
                continue;
            }

            std::cin >> prop1 >> prop2;
            int cash1, cash2;
            std::shared_ptr<OwnableProperty> property;
            std::string group;
            if (std::istringstream iss{prop1}; iss>>cash1) { // Offering cash
                if (bank->checkSufficientFunds(curPlayer->getName(), cash1)) { // Has sufficient funds
                    if(std::istringstream iss{prop2}; iss>>cash2) { // Trying to trade cash for cash
                        std::cout << "You can't trade cash for cash" << std::endl;
                        continue;
                    } else { // Trading cash for property
                        if (bank->getPropertyOwner(prop2) != name) { 
                            std::cout << name << " does not own " << prop2 << std::endl;
                            continue;
                        } else {
                            // If property has improvements on it or if any property in the monopoly has improvements on it
                            // Then it can't be traded
                            property = bank->getProperty(prop2).lock();
                            group = property->getGroup();
                            if (!(group == "Residence" || group == "Gym" || bank->countImprovements(group) == 0)) { 
                                std::cout << "Properties in a monopoly that has improvements on it can't be traded" << std::endl;
                                continue;
                            } else { // Transfers property from curplayer to name
                                std::cout << "You are now the owner of " << prop2 << std::endl;
                                bank->transferFunds(curPlayer->getName(), name, cash1);
                                bank->transferProperty(curPlayer->getName(), prop2);
                            }
                        }
                    }
                } else { // Doesn't have sufficient funds
                    std::cout << "You don't have sufficient funds to offer that amount" << std::endl;
                    std::cout << "You have: $" << curPlayer->getWallet() << std::endl;
                    std::cout << "You offered: $" << cash1 << std::endl;
                    continue;
                }
            } else { // Offering property
                if (bank->getPropertyOwner(prop1) != curPlayer->getName()) { // current player doesn't own property they offered
                    std::cout << "You don't own this property" << std::endl;
                    continue;
                
                } else { // Current player owns the property they offered
                    // If property has improvements on it or if any property in the monopoly has improvements on it
                    // Then it can't be traded
                    property = bank->getProperty(prop1).lock();
                    group = property->getGroup();
                    if (!(group == "Residence" || group == "Gym" || bank->countImprovements(group) == 0)) {
                        std::cout << "Properties in a monopoly that has improvements on it can't be traded" << std::endl;
                        continue;
                    }

                    if (std::istringstream iss{prop2}; iss>>cash2) { // Trades property for cash
                        if (bank->checkSufficientFunds(name, cash2)) {
                            std::cout << "You successfully sold " << prop1 << "for $" << cash2 << std::endl;
                            bank->transferFunds(name, curPlayer->getName(), cash2);
                            bank->transferProperty(name, prop1);
                        } else {
                            std::cout << name << " has insufficient funds to do the deal" << std::endl;
                            continue;
                        }
                    } else { // Trades property for property
                        property = bank->getProperty(prop2).lock();
                        group = property->getGroup();
                        if (!(group == "Residence" || group == "Gym" || bank->countImprovements(group) == 0)) {
                            std::cout << "Properties in a monopoly that has improvements on it can't be traded" << std::endl;
                            continue;
                        } 
                        
                        if (bank->getPropertyOwner(prop2) != name) {
                            std::cout << name << " does not own " << prop2 << std::endl;
                            continue;
                        } else {
                            std::cout << "You successfully traded " << prop1 << " for " << prop2 << std::endl;
                            bank->transferProperty(curPlayer->getName(), prop2);
                            bank->transferProperty(name, prop2);
                        }
                    }
                }
            }
            
        } else if (cmd == "improve") {
            // Lets the player add/sell improvements on their property
            std::string action;
            std::cin >> prop1 >> action;

            if (bank->getPropertyOwner(prop1) != curPlayer->getName()) {
                std::cout << "You do not own this property" << std::endl;
                continue;
            }

          
            std::shared_ptr<AcademicBuilding> ab = std::dynamic_pointer_cast<AcademicBuilding>(bank->getProperty(prop1).lock());
            if (ab->getGroup() == "Residence" || ab->getGroup() == "Gym") {
                std::cout << "You can't add improvements to Residences or Gyms"  << std::endl;
                continue;
            }

            if (action == "buy") {
                if (ab->getImpCount() == 5) {
                    std::cout << "You have the maximum number of improvements for " << prop1 << std::endl;
                    continue;
                }

                if (!bank->transferFunds(curPlayer->getName(), "BANK", ab->getImpCost())) {
                    std::cout << "You don't have sufficient funds to improve your property" << std::endl;
                    std::cout << "You have $" << curPlayer->getWallet() << std::endl;
                    std::cout << "You need $" << ab->getImpCost() << " to improve your property" << std::endl;
                    continue;
                }

                std::cout << "You added an improvement to " << prop1 << std::endl;
                ab->addImps(1);

            } else if (action == "sell") {
                bank->transferFunds("BANK", curPlayer->getName(), ab->getImpCost()*(0.5));
                std::cout << "You have sold an improvement from " << prop1 << std::endl;
                ab->addImps(-1);
            } else {
                std::cout << "Unrecognized command : enter \"help\" to see the list of possible commands" << std::endl;
            }

        } else if (cmd == "mortgage") {
            // Lets the player mortgage their property
            std::cin >> prop1;
            bank->mortgageProperty(prop1, curPlayer->getName());
            

        } else if (cmd == "unmortgage") {
            // Lets the player unmortgage their property (must be able to afford it)
            std::cin >> prop1;
            bank->unmortgageProperty(prop1, curPlayer->getName());

        } else if (cmd == "bankrupt") {
            // Player declares bankrupcy, can only be called if the player is required to pay a fee and can't afford it
            // Checks the total number of players left after removing bankrupt player (if numPlayers == 1 : end game)
            if (!curPlayer->hasToPay()) {
                std::cout << "There is no fee that you are required to pay. You cannot declare bankrupcy" << std::endl;
                continue;
            }

            bank->removePlayer(curPlayer->getName());
            this->removePlayer(*curPlayer);
            if (players.size() == 1) {
                std::cout << "The game is over!" << std::endl;
                std::cout << players[0]->getName() << " wins!" << std::endl;
            }

        } else if (cmd == "assets") {
            // Prints out the current player's assets
            // Does not work if a player is must pay Tuition fee
            if (curPlayer->hasToPay() && this->getTileName(curPlayer->getPosition()) == "TUITION") {
                std::cout << "You can't look at your assets when paying Tuition" << std::endl;
            } else {
                bank->printAssets(curPlayer->getName());
            }

        } else if (cmd == "all") {
            // Prints out all of the player's assets
            // Does not work if current player must pay Tuition fee
            if (curPlayer->hasToPay() && this->getTileName(curPlayer->getPosition()) == "TUITION") {
                std::cout << "You can't look at your assets when paying Tuition" << std::endl;
            } else {
                for (auto p : players) {
                    bank->printAssets(p->getName());
                }
            }

        } else if (cmd == "save") {
            // Saves the game into a load file
            if (curPlayer->hasToPay()) {
                std::cout << "Pay your fee before saving the game" << std::endl;
                continue;
            }
            this->saveGame();
            std::cout << "The game has been saved" << std::endl;
            break;
        } else if (cmd == "help") {
            // Prints out the list of available commands
            for (auto c : commands) {
                std::cout << c << std::endl;
            }

        } else {
            std::cout << "Unrecognized command : enter \"help\" to see the list of possible commands" << std::endl;
        }
    }
}

void Board::movePlayer(Player &p, int roll) {
    int index = p.getPosition();
    const int boardSize = 40;
    index = (index + roll) % boardSize;
    p.setPosition(index);
}

void Board::removePlayer(Player &player) {
    for (auto it = players.begin(); it != players.end(); ++it) {
        if (it->get() == &player) {
            players.erase(it);
            break;
        }
    }
    bank->removePlayer(player.getName());
}

void Board::nextTurn() {
    playerTurn = (playerTurn + 1) % players.size();
}

void Board::notify(Player &p) {
    buildings[p.getPosition()]->performAction(p, *bank);
}

void Board::printBoard(TextDisplay &t) {
    t.printBoard();
}


std::string Board::getTileName(const int n) const {
    return buildings[n]->getName();
}

int Board::getPropCost(const int n) const {
    return buildings[n]->getCost();
}


bool Board::playerExists(std::string name) {
    for (const auto &p : players) {
        if (p->getName() == name) return true;
    }
    return false;
}