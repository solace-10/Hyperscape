// Copyright 2014 Pedro Nunes
//
// This file is part of Nullscape.
//
// Nullscape is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Nullscape is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Nullscape. If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include <memory>
#include <scene/sceneobject.h>
#include <gui/gui.h>

namespace Genesis
{
	class ResourceModel;
	class ResourceSound;
}

namespace Nullscape
{

class Ship;
class ModuleInfo;
class ModuleDetails;
class PanelDocking;
class PanelShipStats;
class PanelShipyard;

typedef std::unique_ptr<PanelDocking> PanelDockingUniquePtr;

#ifdef DEV_MODE_ALLOWED
class PanelShipyardDebug;
#endif

class Shipyard: public Genesis::SceneObject
{
public:
									Shipyard( const glm::vec3& spawnPosition );
	virtual							~Shipyard();

	virtual void					Update( float delta );
	virtual void					Render();

	bool							Dock( Ship* pShip );
	bool							Undock();

	const glm::vec3&				GetPosition() const;

	float							GetDockingRange() const;

	void							SetGrabbedModule( ModuleInfo* pModuleInfo );
	ModuleInfo*						GetGrabbedModule() const;

	void							SetModuleDetails( ModuleInfo* pModuleInfo );

	bool							CanBeUsed() const;

#ifdef DEV_MODE_ALLOWED
	void							LoadFromFile( const std::string& filename );
	void							SaveToFile( const std::string& filename );
	void							ToggleDebugMode();
#endif

private:
	void							InitialiseModels();
	void							SetConstructionDimensions();
	void							UpdateSelection();
	void							UpdateInput();

	bool							ValidateDockedShip();
	void							StoreHexGrid();

    void                            ProvideContextualTip();

	void							LoadSFX();
	void							PlaySFX( Genesis::ResourceSound* pSFX );

	Ship*							m_pDockedShip;
	Genesis::ResourceModel*			m_pShipyardModel;
	Genesis::ResourceModel*			m_pBaseModel;
	glm::vec3						m_Position;
	int								m_SelectedX;
	int								m_SelectedY;
	unsigned int					m_MaxY;
	float							m_BaseModelShowTimer;
	PanelDockingUniquePtr			m_pPanelDocking;
	PanelShipyard*					m_pPanel;
	PanelShipStats*					m_pPanelShipStats;
	ModuleDetails*					m_pModuleDetails;

	ModuleInfo*						m_pGrabbedModule;

	float							m_DockingRange;

	unsigned int					m_MinConstructionX;
	unsigned int					m_MaxConstructionX;
	unsigned int					m_MinConstructionY;
	unsigned int					m_MaxConstructionY;

	Genesis::ResourceSound*			m_pAssemblySFX;
	Genesis::ResourceSound*			m_pDisassemblySFX;

#ifdef DEV_MODE_ALLOWED
	PanelShipyardDebug*				m_pPanelDebug;
	Genesis::InputCallbackToken		m_DebugKeyPressedToken;
#endif
};

inline const glm::vec3& Shipyard::GetPosition() const
{
	return m_Position;
}

inline float Shipyard::GetDockingRange() const
{
	return m_DockingRange;
}

inline ModuleInfo* Shipyard::GetGrabbedModule() const
{
	return m_pGrabbedModule;
}

}