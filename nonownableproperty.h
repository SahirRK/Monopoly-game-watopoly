#ifndef _NONOWNABLEPROPERTY_H_
#define _NONOWNABLEPROPERTY_H_

#include "tile.h"
#include "bank.h"


class NonOwnableProperty : public Tile {
  public:
    NonOwnableProperty(std::string name, int loc);
    virtual void performAction(Player &p, Bank &b) = 0;
};


class CollectOsap : public NonOwnableProperty {
  public:
    CollectOsap(std::string name, int loc);
    void performAction(Player &p, Bank &b) override; 
};

class DCTims : public NonOwnableProperty {
  public:
    DCTims(std::string name, int loc);
    void performAction(Player &p, Bank &b) override;
};

class GoToTims : public NonOwnableProperty {
  public:
    GoToTims(std::string name, int loc);
    void performAction(Player &p, Bank &b) override;
};

class GooseNesting : public NonOwnableProperty {
  public:
    GooseNesting(std::string name, int loc);
    void performAction(Player &p, Bank &b) override;
};

class Tuition : public NonOwnableProperty {
  public:
    Tuition(std::string name, int loc);
    void performAction(Player &p, Bank &b) override;
};

class CoopFee : public NonOwnableProperty {
  public:
    CoopFee(std::string name, int loc);
    void performAction(Player &p, Bank &b) override;
};

class SLC : public NonOwnableProperty {
  public:
    SLC(std::string name, int loc);
    void performAction(Player &p, Bank &b) override;
};

class NH : public NonOwnableProperty {
  public:
    NH(std::string name, int loc);
    void performAction(Player &p, Bank &b) override;
};

#endif // NONOWNABLEPROPERTY_H
