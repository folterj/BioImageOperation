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

#pragma once


/*
 * Windows Form for text output
 */

namespace BioImageOperation {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	public ref class InfoForm : public System::Windows::Forms::Form
	{
		/// <summary>
		/// Required designer variable.
		/// </summary>
	private: System::ComponentModel::IContainer^  components;
	private: System::Windows::Forms::RichTextBox^  infoText;
	private: System::Windows::Forms::ContextMenuStrip^  contextMenuStrip;
	private: System::Windows::Forms::ToolStripMenuItem^  copyToolStripMenuItem;

	public:
		int title;
		bool rtfMode = false;

		InfoForm(void);
		void setTitle(int title);
		void showInfo(System::String^ info);
		void showRtf(System::String^ rtf);

	protected:
		~InfoForm();

	private:
		System::Void InfoForm_FormClosing(System::Object^  sender, System::Windows::Forms::FormClosingEventArgs^  e);
		System::Void copyToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e);

#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			this->components = (gcnew System::ComponentModel::Container());
			System::ComponentModel::ComponentResourceManager^  resources = (gcnew System::ComponentModel::ComponentResourceManager(InfoForm::typeid));
			this->infoText = (gcnew System::Windows::Forms::RichTextBox());
			this->contextMenuStrip = (gcnew System::Windows::Forms::ContextMenuStrip(this->components));
			this->copyToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->contextMenuStrip->SuspendLayout();
			this->SuspendLayout();
			// 
			// infoText
			// 
			this->infoText->ContextMenuStrip = this->contextMenuStrip;
			this->infoText->Dock = System::Windows::Forms::DockStyle::Fill;
			this->infoText->Location = System::Drawing::Point(0, 0);
			this->infoText->Name = L"infoText";
			this->infoText->ReadOnly = true;
			this->infoText->Size = System::Drawing::Size(284, 261);
			this->infoText->TabIndex = 0;
			this->infoText->Text = L"";
			// 
			// contextMenuStrip
			// 
			this->contextMenuStrip->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(1) { this->copyToolStripMenuItem });
			this->contextMenuStrip->Name = L"contextMenuStrip";
			this->contextMenuStrip->Size = System::Drawing::Size(103, 26);
			// 
			// copyToolStripMenuItem
			// 
			this->copyToolStripMenuItem->Name = L"copyToolStripMenuItem";
			this->copyToolStripMenuItem->Size = System::Drawing::Size(102, 22);
			this->copyToolStripMenuItem->Text = L"Copy";
			this->copyToolStripMenuItem->Click += gcnew System::EventHandler(this, &InfoForm::copyToolStripMenuItem_Click);
			// 
			// InfoForm
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(284, 261);
			this->ContextMenuStrip = this->contextMenuStrip;
			this->Controls->Add(this->infoText);
			this->Icon = (cli::safe_cast<System::Drawing::Icon^>(resources->GetObject(L"$this.Icon")));
			this->Name = L"InfoForm";
			this->SizeGripStyle = System::Windows::Forms::SizeGripStyle::Show;
			this->Text = L"BIO Info";
			this->FormClosing += gcnew System::Windows::Forms::FormClosingEventHandler(this, &InfoForm::InfoForm_FormClosing);
			this->contextMenuStrip->ResumeLayout(false);
			this->ResumeLayout(false);

		}
#pragma endregion

	};
}
