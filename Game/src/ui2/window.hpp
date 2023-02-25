// Copyright 2023 Pedro Nunes
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

#include <string>

#include <coredefines.h>

namespace Nullscape::UI2
{

class Window
{
public:
	Window(const std::string& title);
	virtual ~Window() {};

	virtual void Update(float delta);
};
GENESIS_DECLARE_SMART_PTR( Window );

} // namespace Nullscape::UI2