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

#include "InfoForm.h"

using namespace BioImageOperation;


InfoForm::InfoForm(void)
{
	InitializeComponent();
}

InfoForm::~InfoForm()
{
	if (components)
	{
		delete components;
	}
}

void InfoForm::setTitle(int title)
{
	this->title = title;
	Text = "BIO Info " + title.ToString();
}

void InfoForm::showInfo(System::String^ info)
{
	infoText->Text = info;
	if (!Visible)
	{
		Show();
	}
}

void InfoForm::showRtf(System::String^ rtf)
{
	infoText->Rtf = rtf;
	rtfMode = true;
	if (!Visible)
	{
		Show();
	}
}

System::Void InfoForm::InfoForm_FormClosing(System::Object^  sender, System::Windows::Forms::FormClosingEventArgs^  e)
{
	e->Cancel = true;
	Hide();
}

System::Void InfoForm::copyToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e)
{
	if (rtfMode)
	{
		Clipboard::SetData(System::Windows::Forms::DataFormats::Rtf, infoText->Rtf);
	}
	else
	{
		Clipboard::SetText(infoText->Text);
	}
}
