using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace FlagsCalculator
{
    public partial class Form1 : Form
    {
        private const Byte RFLAG_LENGTH = 64;
        private const Byte EFLAG_LENGTH = 32;
        private CheckBox[] bitBoxList=new CheckBox[RFLAG_LENGTH];
        public Form1()
        {
            InitializeComponent();
            InitSubscribeList();
        }
        
        private void button1_Click(Object sender, EventArgs e)
        {
            try
            {
                String result = Convert16To2(textBox1.Text);
                for (int i = 0; i < result.Length; i++)
                {
                    if (bitBoxList[i] != null && result[i] == '0')
                    {
                        bitBoxList[i].Checked = false;
                    }
                    else if (bitBoxList[i] != null && result[i] == '1')
                    {
                        bitBoxList[i].Checked = true;
                    }
                }
            }
            catch
            {
                ClearAll();
                MessageBox.Show("Error Happened. Please input a valid number");
            }
        }
        private void SubscribeBit(Byte bit, CheckBox bitBox)
        {
            bitBoxList[EFLAG_LENGTH - bit -1] = bitBox;
        }

        private String Convert16To2(String hexStr)
        {
            int value = Convert.ToInt32(hexStr, 16);
            String result = Convert.ToString(value, 2).PadLeft(32, '0');
            return result;
        }

        private void InitSubscribeList()
        {
            SubscribeBit(21, checkBox1);//ID
            SubscribeBit(20, checkBox2);//VIP
            SubscribeBit(19, checkBox3);//VIF
            SubscribeBit(18, checkBox4);//AC
            SubscribeBit(17, checkBox5);//VM

            SubscribeBit(16, checkBox6);//RF
            SubscribeBit(14, checkBox7);//NT
            SubscribeBit(13, checkBox8);//IOPL
            SubscribeBit(12, checkBox18);//IOPL
            SubscribeBit(11, checkBox9);//OF

            SubscribeBit(10, checkBox10);//DF
            SubscribeBit(9, checkBox11);//IF
            SubscribeBit(8, checkBox12);//TF
            SubscribeBit(7, checkBox13);//SF
            SubscribeBit(6, checkBox14);//ZF

            SubscribeBit(4, checkBox15);//AF
            SubscribeBit(2, checkBox16);//PF
            SubscribeBit(0, checkBox17);//CF
        }

        private void DisplayAll()
        {
            for (int i = 0; i < bitBoxList.Length; i++)
            {
                if (bitBoxList[i] != null)
                {
                    bitBoxList[i].Visible = true;
                }
            }
        }
        private void radioButton3_CheckedChanged(object sender, EventArgs e)
        {
            this.DisplayAll();

            checkBox9.Visible = false;//OF

            checkBox10.Visible = false;//DF
            checkBox13.Visible = false;//SF
            checkBox14.Visible = false;//ZF

            checkBox15.Visible = false;//AF
            checkBox16.Visible = false;//PF
            checkBox17.Visible = false;//CF
        }

        private void radioButton4_CheckedChanged(object sender, EventArgs e)
        {
            this.DisplayAll();
        }

        private void radioButton2_CheckedChanged(object sender, EventArgs e)
        {
            this.DisplayAll();
            checkBox1.Visible = false;//ID
            checkBox2.Visible = false;//VIP
            checkBox3.Visible = false;//VIF
            checkBox4.Visible = false;//AC
            checkBox5.Visible = false;//VM

            checkBox6.Visible = false;//ID
            checkBox7.Visible = false;//VIP
            checkBox8.Visible = false;//VIF
            checkBox9.Visible = false;//ID
            checkBox11.Visible = false;//ID
            checkBox12.Visible = false;//VIP
            checkBox13.Visible = false;//VIF
            checkBox14.Visible = false;//AC
            checkBox15.Visible = false;//VM

            checkBox16.Visible = false;//ID
            checkBox17.Visible = false;//VIP
            checkBox18.Visible = false;//VIF
            
        }

        private void radioButton1_CheckedChanged(object sender, EventArgs e)
        {
            this.DisplayAll();
            checkBox1.Visible = false;//ID
            checkBox2.Visible = false;//VIP
            checkBox3.Visible = false;//VIF
            checkBox4.Visible = false;//AC
            checkBox5.Visible = false;//VM

            checkBox6.Visible = false;//ID
            checkBox7.Visible = false;//VIP
            checkBox8.Visible = false;//VIF
            checkBox11.Visible = false;//ID
            checkBox12.Visible = false;//VIP
            checkBox18.Visible = false;//VIP
        }

        private void textBox1_TextChanged(object sender, EventArgs e)
        {
            if (textBox1.Text == "")
            {
                button1.Enabled = false;
            }
            else
            {
                button1.Enabled = true;
            }
        }

        private void ClearAll()
        {
            for (int i = 0; i < EFLAG_LENGTH; i++)
            {
                if (bitBoxList[i] != null)
                {
                    bitBoxList[i].Checked = false;
                }
            }
        }
    }
}
