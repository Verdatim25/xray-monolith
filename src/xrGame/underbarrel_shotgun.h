///////////////////////////////////////////////////////////////
// underbarrel_shotgun.h
// underbarrel_shotgun - new addont for underbarrel_shotgun
// code mostly copied from GrenadeLauncher.h
///////////////////////////////////////////////////////////////

#pragma once
#include "inventory_item_object.h"

class CAddonShotgun : public CInventoryItemObject
{
private:
    typedef CInventoryItemObject inherited;
public:
    CAddonShotgun(void);
    virtual ~CAddonShotgun(void);

    virtual BOOL net_Spawn(CSE_Abstract* DC);
    virtual void Load(LPCSTR section);
    virtual void net_Destroy();

    virtual void OnH_A_Chield();
    virtual void OnH_B_Independent(bool just_before_destroy);

    virtual void UpdateCL();
    virtual void renderable_Render();

    float GetGrenadeVel() { return m_fGrenadeVel; }

protected:
    //стартовая скорость вылета подствольной гранаты
    float m_fGrenadeVel;
};
