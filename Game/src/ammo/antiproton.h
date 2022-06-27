// Copyright 2020 Pedro Nunes
//
// This file is part of Hexterminate.
//
// Hexterminate is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Hexterminate is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Hexterminate. If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include <resources/resourceshader.hpp>

#include "ammo/ammo.h"

namespace Genesis
{
	class ResourceImage;
    class VertexBuffer;
}

namespace Hexterminate
{

class Weapon;


///////////////////////////////////////////////////////////////////////////////
// Antiproton
///////////////////////////////////////////////////////////////////////////////

class Antiproton: public Ammo
{
public:
										Antiproton();
	virtual								~Antiproton();

	virtual void						Create( Weapon* pWeapon, float additionalRotation = 0.0f  ) override;
	virtual void						Update( float delta ) override;
	virtual void						Render() override;

	QuantumState						GetQuantumState() const;

private:
	static Genesis::ResourceShader*		m_pShader;
    Genesis::VertexBuffer*              m_pVertexBuffer;
	Genesis::ShaderUniformSharedPtr		m_pInternalRadiusUniform;
	Genesis::ShaderUniformSharedPtr		m_pExternalRadiusUniform;
	Genesis::ShaderUniformSharedPtr		m_pQuantumStateUniform;
	QuantumState						m_QuantumState;
};

}