#pragma once

#include<stdio.h>

namespace VideoAnalyzer {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	/// <summary>
	/// Summary for Form1
	/// </summary>
	public ref class Form1 : public System::Windows::Forms::Form
	{
	public:
        System::String^ mfilename;
		Form1(void)
		{
			InitializeComponent();
			//
			//TODO: Add the constructor code here
			//
		}

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~Form1()
		{
			if (components)
			{
				delete components;
			}
		}
    private: System::Windows::Forms::MenuStrip^  menuStrip1;
    protected: 
    private: System::Windows::Forms::ToolStripMenuItem^  fileToolStripMenuItem;
    private: System::Windows::Forms::ToolStripMenuItem^  openToolStripMenuItem;
    private: System::Windows::Forms::ToolStripMenuItem^  closeToolStripMenuItem;
    private: System::Windows::Forms::ToolStripMenuItem^  exitToolStripMenuItem;
    private: System::Windows::Forms::ToolStripMenuItem^  settingToolStripMenuItem;
    private: System::Windows::Forms::ToolStripMenuItem^  windowsToolStripMenuItem;
    private: System::Windows::Forms::ToolStripMenuItem^  bitRateWindowToolStripMenuItem;
    private: System::Windows::Forms::ToolStripMenuItem^  vBVBufferWindowToolStripMenuItem;
    private: System::Windows::Forms::ToolStripMenuItem^  pSNRWindowToolStripMenuItem;
    private: System::Windows::Forms::ToolStripMenuItem^  aboutToolStripMenuItem;
    private: System::Windows::Forms::ToolStripMenuItem^  versionToolStripMenuItem;

    private: System::Windows::Forms::OpenFileDialog^  openFileDialog1;
    private: System::Windows::Forms::Panel^  VideoInfoPannel;

    private: System::Windows::Forms::Label^  label1;
    private: System::Windows::Forms::Label^  label3;
    private: System::Windows::Forms::Label^  label2;
    private: System::Windows::Forms::Panel^  VideoPlaybackPannel;
    private: System::Windows::Forms::Panel^  VideoBitratePannel;
    private: System::Windows::Forms::Panel^  VBVBufferPannel;




    protected: 




    private: System::ComponentModel::IContainer^  components;

	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>


#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
            System::ComponentModel::ComponentResourceManager^  resources = (gcnew System::ComponentModel::ComponentResourceManager(Form1::typeid));
            this->menuStrip1 = (gcnew System::Windows::Forms::MenuStrip());
            this->fileToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
            this->openToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
            this->closeToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
            this->exitToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
            this->settingToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
            this->windowsToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
            this->bitRateWindowToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
            this->vBVBufferWindowToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
            this->pSNRWindowToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
            this->aboutToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
            this->versionToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
            this->openFileDialog1 = (gcnew System::Windows::Forms::OpenFileDialog());
            this->VideoInfoPannel = (gcnew System::Windows::Forms::Panel());
            this->label3 = (gcnew System::Windows::Forms::Label());
            this->label2 = (gcnew System::Windows::Forms::Label());
            this->label1 = (gcnew System::Windows::Forms::Label());
            this->VideoPlaybackPannel = (gcnew System::Windows::Forms::Panel());
            this->VideoBitratePannel = (gcnew System::Windows::Forms::Panel());
            this->VBVBufferPannel = (gcnew System::Windows::Forms::Panel());
            this->menuStrip1->SuspendLayout();
            this->VideoInfoPannel->SuspendLayout();
            this->SuspendLayout();
            // 
            // menuStrip1
            // 
            this->menuStrip1->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(4) {this->fileToolStripMenuItem, 
                this->settingToolStripMenuItem, this->windowsToolStripMenuItem, this->aboutToolStripMenuItem});
            this->menuStrip1->Location = System::Drawing::Point(0, 0);
            this->menuStrip1->Name = L"menuStrip1";
            this->menuStrip1->Size = System::Drawing::Size(985, 25);
            this->menuStrip1->TabIndex = 0;
            this->menuStrip1->Text = L"menuStrip1";
            // 
            // fileToolStripMenuItem
            // 
            this->fileToolStripMenuItem->DropDownItems->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(3) {this->openToolStripMenuItem, 
                this->closeToolStripMenuItem, this->exitToolStripMenuItem});
            this->fileToolStripMenuItem->Name = L"fileToolStripMenuItem";
            this->fileToolStripMenuItem->Size = System::Drawing::Size(39, 21);
            this->fileToolStripMenuItem->Text = L"File";
            // 
            // openToolStripMenuItem
            // 
            this->openToolStripMenuItem->Name = L"openToolStripMenuItem";
            this->openToolStripMenuItem->Size = System::Drawing::Size(108, 22);
            this->openToolStripMenuItem->Text = L"Open";
            this->openToolStripMenuItem->Click += gcnew System::EventHandler(this, &Form1::openToolStripMenuItem_Click);
            // 
            // closeToolStripMenuItem
            // 
            this->closeToolStripMenuItem->Name = L"closeToolStripMenuItem";
            this->closeToolStripMenuItem->Size = System::Drawing::Size(108, 22);
            this->closeToolStripMenuItem->Text = L"Close";
            // 
            // exitToolStripMenuItem
            // 
            this->exitToolStripMenuItem->Name = L"exitToolStripMenuItem";
            this->exitToolStripMenuItem->Size = System::Drawing::Size(108, 22);
            this->exitToolStripMenuItem->Text = L"Exit";
            // 
            // settingToolStripMenuItem
            // 
            this->settingToolStripMenuItem->Name = L"settingToolStripMenuItem";
            this->settingToolStripMenuItem->Size = System::Drawing::Size(60, 21);
            this->settingToolStripMenuItem->Text = L"Setting";
            // 
            // windowsToolStripMenuItem
            // 
            this->windowsToolStripMenuItem->DropDownItems->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(3) {this->bitRateWindowToolStripMenuItem, 
                this->vBVBufferWindowToolStripMenuItem, this->pSNRWindowToolStripMenuItem});
            this->windowsToolStripMenuItem->Name = L"windowsToolStripMenuItem";
            this->windowsToolStripMenuItem->Size = System::Drawing::Size(73, 21);
            this->windowsToolStripMenuItem->Text = L"Windows";
            // 
            // bitRateWindowToolStripMenuItem
            // 
            this->bitRateWindowToolStripMenuItem->CheckOnClick = true;
            this->bitRateWindowToolStripMenuItem->Name = L"bitRateWindowToolStripMenuItem";
            this->bitRateWindowToolStripMenuItem->Size = System::Drawing::Size(190, 22);
            this->bitRateWindowToolStripMenuItem->Text = L"BitRate Window";
            // 
            // vBVBufferWindowToolStripMenuItem
            // 
            this->vBVBufferWindowToolStripMenuItem->CheckOnClick = true;
            this->vBVBufferWindowToolStripMenuItem->Name = L"vBVBufferWindowToolStripMenuItem";
            this->vBVBufferWindowToolStripMenuItem->Size = System::Drawing::Size(190, 22);
            this->vBVBufferWindowToolStripMenuItem->Text = L"VBV Buffer Window";
            // 
            // pSNRWindowToolStripMenuItem
            // 
            this->pSNRWindowToolStripMenuItem->CheckOnClick = true;
            this->pSNRWindowToolStripMenuItem->Name = L"pSNRWindowToolStripMenuItem";
            this->pSNRWindowToolStripMenuItem->Size = System::Drawing::Size(190, 22);
            this->pSNRWindowToolStripMenuItem->Text = L"PSNR Window";
            // 
            // aboutToolStripMenuItem
            // 
            this->aboutToolStripMenuItem->DropDownItems->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(1) {this->versionToolStripMenuItem});
            this->aboutToolStripMenuItem->Name = L"aboutToolStripMenuItem";
            this->aboutToolStripMenuItem->Size = System::Drawing::Size(55, 21);
            this->aboutToolStripMenuItem->Text = L"About";
            // 
            // versionToolStripMenuItem
            // 
            this->versionToolStripMenuItem->Name = L"versionToolStripMenuItem";
            this->versionToolStripMenuItem->Size = System::Drawing::Size(120, 22);
            this->versionToolStripMenuItem->Text = L"Version";
            // 
            // openFileDialog1
            // 
            this->openFileDialog1->FileName = L"openFileDialog1";
            this->openFileDialog1->InitialDirectory = L"D:\\";
            // 
            // VideoInfoPannel
            // 
            this->VideoInfoPannel->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom) 
                | System::Windows::Forms::AnchorStyles::Left));
            this->VideoInfoPannel->AutoScroll = true;
            this->VideoInfoPannel->AutoSize = true;
            this->VideoInfoPannel->BackColor = System::Drawing::SystemColors::Control;
            this->VideoInfoPannel->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
            this->VideoInfoPannel->Controls->Add(this->label3);
            this->VideoInfoPannel->Controls->Add(this->label2);
            this->VideoInfoPannel->Controls->Add(this->label1);
            this->VideoInfoPannel->Location = System::Drawing::Point(11, 23);
            this->VideoInfoPannel->Name = L"VideoInfoPannel";
            this->VideoInfoPannel->Size = System::Drawing::Size(138, 529);
            this->VideoInfoPannel->TabIndex = 1;
            // 
            // label3
            // 
            this->label3->AutoSize = true;
            this->label3->Location = System::Drawing::Point(9, 134);
            this->label3->Name = L"label3";
            this->label3->Size = System::Drawing::Size(47, 12);
            this->label3->TabIndex = 2;
            this->label3->Text = L"BitRate";
            // 
            // label2
            // 
            this->label2->AutoSize = true;
            this->label2->Location = System::Drawing::Point(10, 66);
            this->label2->Name = L"label2";
            this->label2->Size = System::Drawing::Size(35, 12);
            this->label2->TabIndex = 1;
            this->label2->Text = L"Codec";
            // 
            // label1
            // 
            this->label1->AutoSize = true;
            this->label1->Location = System::Drawing::Point(10, 16);
            this->label1->Name = L"label1";
            this->label1->Size = System::Drawing::Size(65, 12);
            this->label1->TabIndex = 0;
            this->label1->Text = L"Resolution";
            // 
            // VideoPlaybackPannel
            // 
            this->VideoPlaybackPannel->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom) 
                | System::Windows::Forms::AnchorStyles::Left) 
                | System::Windows::Forms::AnchorStyles::Right));
            this->VideoPlaybackPannel->AutoScroll = true;
            this->VideoPlaybackPannel->AutoSize = true;
            this->VideoPlaybackPannel->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
            this->VideoPlaybackPannel->Location = System::Drawing::Point(150, 23);
            this->VideoPlaybackPannel->Name = L"VideoPlaybackPannel";
            this->VideoPlaybackPannel->Size = System::Drawing::Size(834, 361);
            this->VideoPlaybackPannel->TabIndex = 2;
            this->VideoPlaybackPannel->Paint += gcnew System::Windows::Forms::PaintEventHandler(this, &Form1::panel2_Paint);
            // 
            // VideoBitratePannel
            // 
            this->VideoBitratePannel->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Left) 
                | System::Windows::Forms::AnchorStyles::Right));
            this->VideoBitratePannel->AutoScroll = true;
            this->VideoBitratePannel->AutoSize = true;
            this->VideoBitratePannel->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
            this->VideoBitratePannel->Location = System::Drawing::Point(150, 390);
            this->VideoBitratePannel->Name = L"VideoBitratePannel";
            this->VideoBitratePannel->Size = System::Drawing::Size(405, 162);
            this->VideoBitratePannel->TabIndex = 3;
            this->VideoBitratePannel->Paint += gcnew System::Windows::Forms::PaintEventHandler(this, &Form1::VideoBitratePannel_Paint);
            // 
            // VBVBufferPannel
            // 
            this->VBVBufferPannel->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Left) 
                | System::Windows::Forms::AnchorStyles::Right));
            this->VBVBufferPannel->AutoScroll = true;
            this->VBVBufferPannel->AutoSize = true;
            this->VBVBufferPannel->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
            this->VBVBufferPannel->Location = System::Drawing::Point(561, 390);
            this->VBVBufferPannel->Name = L"VBVBufferPannel";
            this->VBVBufferPannel->Size = System::Drawing::Size(422, 162);
            this->VBVBufferPannel->TabIndex = 4;
            this->VBVBufferPannel->Paint += gcnew System::Windows::Forms::PaintEventHandler(this, &Form1::VBVBufferPannel_Paint);
            // 
            // Form1
            // 
            this->AutoScaleDimensions = System::Drawing::SizeF(6, 12);
            this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
            this->ClientSize = System::Drawing::Size(985, 553);
            this->Controls->Add(this->VBVBufferPannel);
            this->Controls->Add(this->VideoBitratePannel);
            this->Controls->Add(this->VideoPlaybackPannel);
            this->Controls->Add(this->VideoInfoPannel);
            this->Controls->Add(this->menuStrip1);
            this->Font = (gcnew System::Drawing::Font(L"SimSun", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
                static_cast<System::Byte>(134)));
            this->Icon = (cli::safe_cast<System::Drawing::Icon^  >(resources->GetObject(L"$this.Icon")));
            this->MainMenuStrip = this->menuStrip1;
            this->Name = L"Form1";
            this->Text = L"VideoAnalyzer";
            this->menuStrip1->ResumeLayout(false);
            this->menuStrip1->PerformLayout();
            this->VideoInfoPannel->ResumeLayout(false);
            this->VideoInfoPannel->PerformLayout();
            this->ResumeLayout(false);
            this->PerformLayout();

        }
#pragma endregion
    private: System::Void openToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e) {
                 openFileDialog1->ShowDialog();
                 mfilename = openFileDialog1->FileName;
                 this->Text = L"VideoAnalyzer " + mfilename;
             }
private: System::Void panel2_Paint(System::Object^  sender, System::Windows::Forms::PaintEventArgs^  e) {
             Graphics^ g = VideoPlaybackPannel->CreateGraphics();
             Pen^ p = gcnew Pen(Color::Black);
             g->DrawLine(p, 0, 0, 50, 50);
             delete g;
         }
private: System::Void VideoBitratePannel_Paint(System::Object^  sender, System::Windows::Forms::PaintEventArgs^  e) {
             Graphics^ g = VideoBitratePannel->CreateGraphics();
             Pen^ p = gcnew Pen(Color::Red);
             g->DrawLine(p, 0, 0, 50, 50);
             delete g;
         }
private: System::Void VBVBufferPannel_Paint(System::Object^  sender, System::Windows::Forms::PaintEventArgs^  e) {
             Graphics^ g = VBVBufferPannel->CreateGraphics();
             Pen^ p = gcnew Pen(Color::Green);
             g->DrawLine(p, 0, 0, 50, 50);
             delete g;
         }
};
}

