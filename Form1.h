#pragma once

#include<stdio.h>
#include<windows.h>
#include<string>
#include "w32thread.h"
#include "queue.h"

enum 
{
    PS_NONE  = 0,
    PS_INIT  = 1,
    PS_PLAY  = 2,
    PS_PAUSE = 3,
    PS_EXIT  = 4,
};

typedef struct VideoPlayer{
    AVCodecContext* avctx;
    PacketQueue     videoq;  // video packet queue
    FrameQueue      pictq;   // video decoded frame queue 
    struct SwsContext *sws_ctx;
    int             frameInterval; // time interval between two frames
    AVRational      time_base;
    int             width;
    int             height;
    long long       playStartTime;  // ms
    long long       playPauseTime;
    int eof;
} VideoPlayer;

namespace VideoAnalyzer {
    using namespace System;
    using namespace System::ComponentModel;
    using namespace System::Collections;
    using namespace System::Windows::Forms;
    using namespace System::Data;
    using namespace System::Drawing;
    using namespace System::Drawing::Drawing2D;
    using namespace System::Drawing::Imaging;
    using namespace System::Threading;
    /// <summary>
    /// Summary for Form1
    /// </summary>
    public ref class Form1 : public System::Windows::Forms::Form
    {
    public:
        System::String^ mfilename;  // input filename
        Form1(void);
        void PlayerInit();
        void PlayerExit();
        Int32 PlayStat;
        static pthread_mutex_t* m_mtxPlayStat;
        static pthread_cond_t*  m_condPlayCond;
        static pthread_mutex_t* m_mtxRender;
        Thread^ readThread;
        Thread^ decThread;
        Thread^ rendThread;
        VideoPlayer* m_pl;
        Bitmap^      m_rpic; // render picture
        AVCodecContext* m_avctx;
        AVCodec* m_avcodec;
        int video_stream_index;
        bool m_doscale;

        Graphics^ m_videoPlayGraphic;

        Int32 m_renderTlx;
        Int32 m_renderTly;
        Int32 m_renderAreaWidth;
        Int32 m_renderAreaHeight;

        delegate void setVideoInfo(String^ str_res);
        setVideoInfo^ mSetVidInfDelegate;

    protected:
        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        ~Form1();

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
    private: System::Windows::Forms::Label^  VideoInfoLabel;

    private: System::Windows::Forms::Panel^  VideoPlaybackPannel;
    private: System::Windows::Forms::Panel^  VideoBitratePannel;
    private: System::Windows::Forms::Panel^  VBVBufferPannel;
    private: System::Windows::Forms::Button^  PlayButton;

    private: System::Windows::Forms::Button^  StopButton;
    private: System::Windows::Forms::Panel^  ControlPannel;

    public: System::Void setVideoInfoMethod(String^ str)
            {
                VideoInfoLabel->Text = L"============Video Info============\n" + str;
            }

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
            this->VideoInfoLabel = (gcnew System::Windows::Forms::Label());
            this->VideoPlaybackPannel = (gcnew System::Windows::Forms::Panel());
            this->VideoBitratePannel = (gcnew System::Windows::Forms::Panel());
            this->VBVBufferPannel = (gcnew System::Windows::Forms::Panel());
            this->PlayButton = (gcnew System::Windows::Forms::Button());
            this->StopButton = (gcnew System::Windows::Forms::Button());
            this->ControlPannel = (gcnew System::Windows::Forms::Panel());
            this->menuStrip1->SuspendLayout();
            this->VideoInfoPannel->SuspendLayout();
            this->ControlPannel->SuspendLayout();
            this->SuspendLayout();
            // 
            // menuStrip1
            // 
            this->menuStrip1->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(4) {this->fileToolStripMenuItem, 
                this->settingToolStripMenuItem, this->windowsToolStripMenuItem, this->aboutToolStripMenuItem});
            this->menuStrip1->Location = System::Drawing::Point(0, 0);
            this->menuStrip1->Name = L"menuStrip1";
            this->menuStrip1->Size = System::Drawing::Size(1035, 25);
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
            this->VideoInfoPannel->BackColor = System::Drawing::SystemColors::Control;
            this->VideoInfoPannel->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
            this->VideoInfoPannel->Controls->Add(this->VideoInfoLabel);
            this->VideoInfoPannel->Location = System::Drawing::Point(13, 23);
            this->VideoInfoPannel->Name = L"VideoInfoPannel";
            this->VideoInfoPannel->Size = System::Drawing::Size(217, 670);
            this->VideoInfoPannel->TabIndex = 1;
            // 
            // VideoInfoLabel
            // 
            this->VideoInfoLabel->AutoSize = true;
            this->VideoInfoLabel->Location = System::Drawing::Point(2, 7);
            this->VideoInfoLabel->Name = L"VideoInfoLabel";
            this->VideoInfoLabel->Size = System::Drawing::Size(209, 12);
            this->VideoInfoLabel->TabIndex = 0;
            this->VideoInfoLabel->Text = L"============Video Info============";
            // 
            // VideoPlaybackPannel
            // 
            //this->DoubleBuffered = true;
            //this->SetStyle(ControlStyles::UserPaint, true);
            //this->SetStyle(ControlStyles::AllPaintingInWmPaint, true); 
            //this->SetStyle(ControlStyles::DoubleBuffer, true);

            this->VideoPlaybackPannel->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom) 
                | System::Windows::Forms::AnchorStyles::Left) 
                | System::Windows::Forms::AnchorStyles::Right));      
            this->VideoPlaybackPannel->AutoScroll = true;
            this->VideoPlaybackPannel->AutoSize = true;
            this->VideoPlaybackPannel->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
            this->VideoPlaybackPannel->Location = System::Drawing::Point(232, 23);
            this->VideoPlaybackPannel->Name = L"VideoPlaybackPannel";
            this->VideoPlaybackPannel->Size = System::Drawing::Size(800, 450);
            this->VideoPlaybackPannel->TabIndex = 2;
            this->VideoPlaybackPannel->Paint += gcnew System::Windows::Forms::PaintEventHandler(this, &Form1::VideoPlaybackPannel_Paint);
            // 
            // VideoBitratePannel
            // 
            this->VideoBitratePannel->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Left) 
                | System::Windows::Forms::AnchorStyles::Right));
            this->VideoBitratePannel->AutoScroll = true;
            this->VideoBitratePannel->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
            this->VideoBitratePannel->Location = System::Drawing::Point(232, 513);
            this->VideoBitratePannel->Name = L"VideoBitratePannel";
            this->VideoBitratePannel->Size = System::Drawing::Size(399, 180);
            this->VideoBitratePannel->TabIndex = 3;
            this->VideoBitratePannel->Paint += gcnew System::Windows::Forms::PaintEventHandler(this, &Form1::VideoBitratePannel_Paint);
            // 
            // VBVBufferPannel
            // 
            this->VBVBufferPannel->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Right));
            this->VBVBufferPannel->AutoScroll = true;
            this->VBVBufferPannel->AutoSize = true;
            this->VBVBufferPannel->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
            this->VBVBufferPannel->Location = System::Drawing::Point(633, 513);
            this->VBVBufferPannel->Name = L"VBVBufferPannel";
            this->VBVBufferPannel->Size = System::Drawing::Size(399, 180);
            this->VBVBufferPannel->TabIndex = 4;
            this->VBVBufferPannel->Paint += gcnew System::Windows::Forms::PaintEventHandler(this, &Form1::VBVBufferPannel_Paint);
            // 
            // PlayButton
            // 
            this->PlayButton->BackgroundImage = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"PlayButton.BackgroundImage")));
            this->PlayButton->BackgroundImageLayout = System::Windows::Forms::ImageLayout::Zoom;
            this->PlayButton->Location = System::Drawing::Point(0, 3);
            this->PlayButton->Name = L"PlayButton";
            this->PlayButton->Size = System::Drawing::Size(54, 35);
            this->PlayButton->TabIndex = 5;
            this->PlayButton->Text = L"Play";
            this->PlayButton->UseVisualStyleBackColor = true;
            this->PlayButton->Click += gcnew System::EventHandler(this, &Form1::PlayButton_Click);
            // 
            // StopButton
            // 
            this->StopButton->Location = System::Drawing::Point(60, 3);
            this->StopButton->Name = L"StopButton";
            this->StopButton->Size = System::Drawing::Size(54, 35);
            this->StopButton->TabIndex = 6;
            this->StopButton->Text = L"Stop";
            this->StopButton->UseVisualStyleBackColor = true;
            this->StopButton->Click += gcnew System::EventHandler(this, &Form1::StopButton_Click);
            // 
            // ControlPannel
            // 
            this->ControlPannel->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Left) 
                | System::Windows::Forms::AnchorStyles::Right));
            this->ControlPannel->Controls->Add(this->StopButton);
            this->ControlPannel->Controls->Add(this->PlayButton);
            this->ControlPannel->Location = System::Drawing::Point(232, 474);
            this->ControlPannel->Name = L"ControlPannel";
            this->ControlPannel->Size = System::Drawing::Size(800, 38);
            this->ControlPannel->TabIndex = 7;
            // 
            // Form1
            // 
            this->AutoScaleDimensions = System::Drawing::SizeF(6, 12);
            this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
            this->ClientSize = System::Drawing::Size(1035, 705);
            this->Controls->Add(this->ControlPannel);
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
            this->ControlPannel->ResumeLayout(false);
            this->ResumeLayout(false);
            this->PerformLayout();
        }
#pragma endregion
    private: System::Void openToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e);
    private: System::Void VideoPlaybackPannel_Paint(System::Object^  sender, System::Windows::Forms::PaintEventArgs^  e);
    private: System::Void VideoBitratePannel_Paint(System::Object^  sender, System::Windows::Forms::PaintEventArgs^  e);
    private: System::Void VBVBufferPannel_Paint(System::Object^  sender, System::Windows::Forms::PaintEventArgs^  e);
    public:  System::Void setRenderArea();
    private: System::Void drawGrid(Graphics^ g, Int32 Width, Int32 Height, Int32 GridSize, Int32 ipadx, Int32 ipady);
    public:  System::Void RenderFrame(void);
    //public:  System::Void SetPlayStartTime(void);
    //public:  System::Void SetPlayPauseTime(void);
    private: System::Void StopButton_Click(System::Object^  sender, System::EventArgs^  e);
    private: System::Void PlayButton_Click(System::Object^  sender, System::EventArgs^  e);
    };
}


