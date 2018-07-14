/*****************************************************************************
 * Bio Image Operation
 * Copyright (C) 2013-2018 Joost de Folter <folterj@gmail.com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *****************************************************************************/

#include "AboutForm.h"

using namespace System::Diagnostics;
using namespace System::IO;
using namespace System::Reflection;
using namespace BioImageOperation;


AboutForm::AboutForm(void)
{
	InitializeComponent();

	Assembly^ assembly = Assembly::GetExecutingAssembly();

	Text = String::Format("About {0}", assembly->GetName()->Name);
	labelProductName->Text = ((AssemblyProductAttribute^)assembly->GetCustomAttributes(AssemblyProductAttribute::typeid, false)[0])->Product;
	labelVersion->Text = String::Format("Version {0}", assembly->GetName()->Version);
	labelCopyright->Text = ((AssemblyCopyrightAttribute^)assembly->GetCustomAttributes(AssemblyCopyrightAttribute::typeid, false)[0])->Copyright;
	labelCompanyName->Text = ((AssemblyCompanyAttribute^)assembly->GetCustomAttributes(AssemblyCompanyAttribute::typeid, false)[0])->Company;
	textBoxDescription->Text = ((AssemblyDescriptionAttribute^)assembly->GetCustomAttributes(AssemblyDescriptionAttribute::typeid, false)[0])->Description;
}

AboutForm::~AboutForm()
{
	if (components)
	{
		delete components;
	}
}

System::Void AboutForm::licenseButton_Click(System::Object^  sender, System::EventArgs^  e)
{
	try
	{
		Assembly^ assembly = Assembly::GetExecutingAssembly();
		Stream^ stream = assembly->GetManifestResourceStream("gpl3.rtf");
		textBoxDescription->LoadFile(stream, RichTextBoxStreamType::RichText);
	}
	catch (...)
	{
		Process::Start("https://www.gnu.org/licenses/");
	}
}

System::Void AboutForm::textBoxDescription_LinkClicked(System::Object^  sender, System::Windows::Forms::LinkClickedEventArgs^  e)
{
	Process::Start(e->LinkText);
}
