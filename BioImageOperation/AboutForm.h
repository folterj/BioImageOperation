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
 * Windows About Form
 */

namespace BioImageOperation {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	public ref class AboutForm : public System::Windows::Forms::Form
	{
	private: System::ComponentModel::Container ^components;
	private: System::Windows::Forms::Label^  labelVersion;
	private: System::Windows::Forms::TableLayoutPanel^  tableLayoutPanel;
	private: System::Windows::Forms::PictureBox^  logoPictureBox;
	private: System::Windows::Forms::Label^  labelProductName;
	private: System::Windows::Forms::Label^  labelCopyright;
	private: System::Windows::Forms::Label^  labelCompanyName;
	private: System::Windows::Forms::RichTextBox^  textBoxDescription;
	private: System::Windows::Forms::Button^  licenseButton;
	private: System::Windows::Forms::Button^  okButton;

	public:
		AboutForm(void);

	protected:
		~AboutForm();

	private:
		System::Void licenseButton_Click(System::Object^  sender, System::EventArgs^  e);
		System::Void textBoxDescription_LinkClicked(System::Object^  sender, System::Windows::Forms::LinkClickedEventArgs^  e);

#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			System::ComponentModel::ComponentResourceManager^  resources = (gcnew System::ComponentModel::ComponentResourceManager(AboutForm::typeid));
			this->labelVersion = (gcnew System::Windows::Forms::Label());
			this->tableLayoutPanel = (gcnew System::Windows::Forms::TableLayoutPanel());
			this->logoPictureBox = (gcnew System::Windows::Forms::PictureBox());
			this->labelProductName = (gcnew System::Windows::Forms::Label());
			this->labelCopyright = (gcnew System::Windows::Forms::Label());
			this->labelCompanyName = (gcnew System::Windows::Forms::Label());
			this->textBoxDescription = (gcnew System::Windows::Forms::RichTextBox());
			this->licenseButton = (gcnew System::Windows::Forms::Button());
			this->okButton = (gcnew System::Windows::Forms::Button());
			this->tableLayoutPanel->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->logoPictureBox))->BeginInit();
			this->SuspendLayout();
			// 
			// labelVersion
			// 
			this->tableLayoutPanel->SetColumnSpan(this->labelVersion, 2);
			this->labelVersion->Dock = System::Windows::Forms::DockStyle::Fill;
			this->labelVersion->Location = System::Drawing::Point(239, 15);
			this->labelVersion->Margin = System::Windows::Forms::Padding(6, 0, 3, 0);
			this->labelVersion->MaximumSize = System::Drawing::Size(0, 17);
			this->labelVersion->Name = L"labelVersion";
			this->labelVersion->Size = System::Drawing::Size(342, 15);
			this->labelVersion->TabIndex = 0;
			this->labelVersion->Text = L"Version";
			this->labelVersion->TextAlign = System::Drawing::ContentAlignment::MiddleLeft;
			// 
			// tableLayoutPanel
			// 
			this->tableLayoutPanel->ColumnCount = 3;
			this->tableLayoutPanel->ColumnStyles->Add((gcnew System::Windows::Forms::ColumnStyle(System::Windows::Forms::SizeType::Percent,
				40)));
			this->tableLayoutPanel->ColumnStyles->Add((gcnew System::Windows::Forms::ColumnStyle(System::Windows::Forms::SizeType::Percent,
				30)));
			this->tableLayoutPanel->ColumnStyles->Add((gcnew System::Windows::Forms::ColumnStyle(System::Windows::Forms::SizeType::Percent,
				30)));
			this->tableLayoutPanel->Controls->Add(this->logoPictureBox, 0, 0);
			this->tableLayoutPanel->Controls->Add(this->labelProductName, 1, 0);
			this->tableLayoutPanel->Controls->Add(this->labelVersion, 1, 1);
			this->tableLayoutPanel->Controls->Add(this->labelCopyright, 1, 2);
			this->tableLayoutPanel->Controls->Add(this->labelCompanyName, 1, 3);
			this->tableLayoutPanel->Controls->Add(this->textBoxDescription, 1, 4);
			this->tableLayoutPanel->Controls->Add(this->licenseButton, 1, 5);
			this->tableLayoutPanel->Controls->Add(this->okButton, 2, 5);
			this->tableLayoutPanel->Dock = System::Windows::Forms::DockStyle::Fill;
			this->tableLayoutPanel->Location = System::Drawing::Point(0, 0);
			this->tableLayoutPanel->Name = L"tableLayoutPanel";
			this->tableLayoutPanel->RowCount = 6;
			this->tableLayoutPanel->RowStyles->Add((gcnew System::Windows::Forms::RowStyle()));
			this->tableLayoutPanel->RowStyles->Add((gcnew System::Windows::Forms::RowStyle()));
			this->tableLayoutPanel->RowStyles->Add((gcnew System::Windows::Forms::RowStyle()));
			this->tableLayoutPanel->RowStyles->Add((gcnew System::Windows::Forms::RowStyle()));
			this->tableLayoutPanel->RowStyles->Add((gcnew System::Windows::Forms::RowStyle()));
			this->tableLayoutPanel->RowStyles->Add((gcnew System::Windows::Forms::RowStyle()));
			this->tableLayoutPanel->Size = System::Drawing::Size(584, 261);
			this->tableLayoutPanel->TabIndex = 1;
			// 
			// logoPictureBox
			// 
			this->logoPictureBox->BackColor = System::Drawing::Color::Transparent;
			this->logoPictureBox->Dock = System::Windows::Forms::DockStyle::Fill;
			this->logoPictureBox->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"logoPictureBox.Image")));
			this->logoPictureBox->Location = System::Drawing::Point(3, 3);
			this->logoPictureBox->Name = L"logoPictureBox";
			this->tableLayoutPanel->SetRowSpan(this->logoPictureBox, 6);
			this->logoPictureBox->Size = System::Drawing::Size(227, 256);
			this->logoPictureBox->SizeMode = System::Windows::Forms::PictureBoxSizeMode::Zoom;
			this->logoPictureBox->TabIndex = 12;
			this->logoPictureBox->TabStop = false;
			// 
			// labelProductName
			// 
			this->tableLayoutPanel->SetColumnSpan(this->labelProductName, 2);
			this->labelProductName->Dock = System::Windows::Forms::DockStyle::Fill;
			this->labelProductName->Location = System::Drawing::Point(239, 0);
			this->labelProductName->Margin = System::Windows::Forms::Padding(6, 0, 3, 0);
			this->labelProductName->MaximumSize = System::Drawing::Size(0, 17);
			this->labelProductName->Name = L"labelProductName";
			this->labelProductName->Size = System::Drawing::Size(342, 15);
			this->labelProductName->TabIndex = 19;
			this->labelProductName->Text = L"Product Name";
			this->labelProductName->TextAlign = System::Drawing::ContentAlignment::MiddleLeft;
			// 
			// labelCopyright
			// 
			this->tableLayoutPanel->SetColumnSpan(this->labelCopyright, 2);
			this->labelCopyright->Dock = System::Windows::Forms::DockStyle::Fill;
			this->labelCopyright->Location = System::Drawing::Point(239, 30);
			this->labelCopyright->Margin = System::Windows::Forms::Padding(6, 0, 3, 0);
			this->labelCopyright->MaximumSize = System::Drawing::Size(0, 17);
			this->labelCopyright->Name = L"labelCopyright";
			this->labelCopyright->Size = System::Drawing::Size(342, 15);
			this->labelCopyright->TabIndex = 21;
			this->labelCopyright->Text = L"Copyright";
			this->labelCopyright->TextAlign = System::Drawing::ContentAlignment::MiddleLeft;
			// 
			// labelCompanyName
			// 
			this->tableLayoutPanel->SetColumnSpan(this->labelCompanyName, 2);
			this->labelCompanyName->Dock = System::Windows::Forms::DockStyle::Fill;
			this->labelCompanyName->Location = System::Drawing::Point(239, 45);
			this->labelCompanyName->Margin = System::Windows::Forms::Padding(6, 0, 3, 0);
			this->labelCompanyName->MaximumSize = System::Drawing::Size(0, 17);
			this->labelCompanyName->Name = L"labelCompanyName";
			this->labelCompanyName->Size = System::Drawing::Size(342, 15);
			this->labelCompanyName->TabIndex = 22;
			this->labelCompanyName->Text = L"Company Name";
			this->labelCompanyName->TextAlign = System::Drawing::ContentAlignment::MiddleLeft;
			// 
			// textBoxDescription
			// 
			this->tableLayoutPanel->SetColumnSpan(this->textBoxDescription, 2);
			this->textBoxDescription->Dock = System::Windows::Forms::DockStyle::Fill;
			this->textBoxDescription->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75F, System::Drawing::FontStyle::Regular,
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
			this->textBoxDescription->Location = System::Drawing::Point(239, 63);
			this->textBoxDescription->Margin = System::Windows::Forms::Padding(6, 3, 3, 3);
			this->textBoxDescription->Name = L"textBoxDescription";
			this->textBoxDescription->ReadOnly = true;
			this->textBoxDescription->Size = System::Drawing::Size(342, 170);
			this->textBoxDescription->TabIndex = 23;
			this->textBoxDescription->TabStop = false;
			this->textBoxDescription->Text = L"Description";
			this->textBoxDescription->LinkClicked += gcnew System::Windows::Forms::LinkClickedEventHandler(this, &AboutForm::textBoxDescription_LinkClicked);
			// 
			// licenseButton
			// 
			this->licenseButton->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Right));
			this->licenseButton->Location = System::Drawing::Point(330, 239);
			this->licenseButton->Name = L"licenseButton";
			this->licenseButton->Size = System::Drawing::Size(75, 20);
			this->licenseButton->TabIndex = 24;
			this->licenseButton->Text = L"&License";
			this->licenseButton->Click += gcnew System::EventHandler(this, &AboutForm::licenseButton_Click);
			// 
			// okButton
			// 
			this->okButton->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Right));
			this->okButton->DialogResult = System::Windows::Forms::DialogResult::OK;
			this->okButton->Location = System::Drawing::Point(506, 239);
			this->okButton->Name = L"okButton";
			this->okButton->Size = System::Drawing::Size(75, 20);
			this->okButton->TabIndex = 25;
			this->okButton->Text = L"&OK";
			// 
			// AboutForm
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(584, 261);
			this->Controls->Add(this->tableLayoutPanel);
			this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedDialog;
			this->Icon = (cli::safe_cast<System::Drawing::Icon^>(resources->GetObject(L"$this.Icon")));
			this->MaximizeBox = false;
			this->MinimizeBox = false;
			this->Name = L"AboutForm";
			this->StartPosition = System::Windows::Forms::FormStartPosition::CenterParent;
			this->Text = L"About";
			this->tableLayoutPanel->ResumeLayout(false);
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->logoPictureBox))->EndInit();
			this->ResumeLayout(false);

		}
#pragma endregion

	};
}
