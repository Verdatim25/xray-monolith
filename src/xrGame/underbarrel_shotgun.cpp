///////////////////////////////////////////////////////////////
// underbarrel_shotgun.cpp
// underbarrel_shotgun - extra addon for an underbarrel shotgun
// code mostly copied from GrenadeLauncher.cpp
///////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "underbarrel_shotgun.h"
//#include "PhysicsShell.h"

CAddonShotgun::CAddonShotgun()
{
    m_fGrenadeVel = 0.f;
}

CAddonShotgun::~CAddonShotgun()
{
}

BOOL CAddonShotgun::net_Spawn(CSE_Abstract* DC)
{
    return (inherited::net_Spawn(DC));
}

void CAddonShotgun::Load(LPCSTR section)
{
    m_fGrenadeVel = pSettings->r_float(section, "grenade_vel");
    inherited::Load(section);
}

void CAddonShotgun::net_Destroy()
{
    inherited::net_Destroy();
}

void CAddonShotgun::UpdateCL()
{
    inherited::UpdateCL();
}


void CAddonShotgun::OnH_A_Chield()
{
    inherited::OnH_A_Chield();
}

void CAddonShotgun::OnH_B_Independent(bool just_before_destroy)
{
    inherited::OnH_B_Independent(just_before_destroy);
}

void CAddonShotgun::renderable_Render()
{
    inherited::renderable_Render();
}
