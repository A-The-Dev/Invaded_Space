#ifndef BOSSTYPE_H

enum BossTypes{Boss1, Boss2, Boss4};

class BossType
{
public:
    BossType();
    BossTypes getBossType() {return type;}
    void setBossType(BossTypes type) {this->type = type;}
private:
    BossTypes type;
};
#endif // BOSSTYPE_H
