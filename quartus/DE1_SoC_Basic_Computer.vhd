library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity DE1_SoC_Basic_Computer is
    port (
        CLOCK_50   : in    std_logic;
        KEY        : in    std_logic_vector(3 downto 0);
        SW         : in    std_logic_vector(9 downto 0);

        LEDR       : out   std_logic_vector(9 downto 0);

        HEX0       : out   std_logic_vector(6 downto 0);
        HEX1       : out   std_logic_vector(6 downto 0);
        HEX2       : out   std_logic_vector(6 downto 0);
        HEX3       : out   std_logic_vector(6 downto 0);
        HEX4       : out   std_logic_vector(6 downto 0);
        HEX5       : out   std_logic_vector(6 downto 0);

        GPIO_0     : inout std_logic_vector(35 downto 0);
        GPIO_1     : inout std_logic_vector(35 downto 0);

        DRAM_DQ    : inout std_logic_vector(15 downto 0);
        DRAM_ADDR  : out   std_logic_vector(12 downto 0);
        DRAM_BA    : out   std_logic_vector(1 downto 0);
        DRAM_CAS_N : out   std_logic;
        DRAM_RAS_N : out   std_logic;
        DRAM_CLK   : out   std_logic;
        DRAM_CKE   : out   std_logic;
        DRAM_CS_N  : out   std_logic;
        DRAM_WE_N  : out   std_logic;
        DRAM_UDQM  : out   std_logic;
        DRAM_LDQM  : out   std_logic
    );
end DE1_SoC_Basic_Computer;


architecture rtl of DE1_SoC_Basic_Computer is

    ------------------------------------------------------------
    -- Nios II system with Avalon HEX_PIO interface
    ------------------------------------------------------------

    component nios_system is
        port (
            clk                         : in    std_logic;
            reset_n                     : in    std_logic;

            ledr_export                 : out   std_logic_vector(9 downto 0);

            zs_addr_from_the_sdram      : out   std_logic_vector(12 downto 0);
            zs_ba_from_the_sdram        : out   std_logic_vector(1 downto 0);
            zs_cas_n_from_the_sdram     : out   std_logic;
            zs_cke_from_the_sdram       : out   std_logic;
            zs_cs_n_from_the_sdram      : out   std_logic;
            zs_dq_to_and_from_the_sdram : inout std_logic_vector(15 downto 0);
            zs_dqm_from_the_sdram       : out   std_logic_vector(1 downto 0);
            zs_ras_n_from_the_sdram     : out   std_logic;
            zs_we_n_from_the_sdram      : out   std_logic;

            i2c_reg_in_port             : in    std_logic_vector(7 downto 0);
            i2c_reg_out_port            : out   std_logic_vector(7 downto 0);

            -- Four BCD digits
            hex_pio_external_connection_export :
                out std_logic_vector(15 downto 0)
        );
    end component;


    ------------------------------------------------------------
    -- SDRAM PLL
    ------------------------------------------------------------

    component sdram_pll is
        port (
            inclk0 : in  std_logic;
            c0     : out std_logic;
            c1     : out std_logic
        );
    end component;


    ------------------------------------------------------------
    -- Internal signals
    ------------------------------------------------------------

    signal system_clock : std_logic;

    signal dram_ba_s  : std_logic_vector(1 downto 0);
    signal dram_dqm_s : std_logic_vector(1 downto 0);

    signal I2Cin  : std_logic_vector(7 downto 0);
    signal I2Cout : std_logic_vector(7 downto 0);

    -- Temperature in BCD
    -- [15:12] tens
    -- [11:8]  units
    -- [7:4]   tenths
    -- [3:0]   hundredths

    signal temperature_bcd : std_logic_vector(15 downto 0);


    ------------------------------------------------------------
    -- BCD to seven-segment decoder
    -- Active-low segments
    ------------------------------------------------------------

    function hex7seg(value : std_logic_vector(3 downto 0))
        return std_logic_vector is
    begin
        case value is
            when "0000" => return "1000000"; -- 0
            when "0001" => return "1111001"; -- 1
            when "0010" => return "0100100"; -- 2
            when "0011" => return "0110000"; -- 3
            when "0100" => return "0011001"; -- 4
            when "0101" => return "0010010"; -- 5
            when "0110" => return "0000010"; -- 6
            when "0111" => return "1111000"; -- 7
            when "1000" => return "0000000"; -- 8
            when "1001" => return "0010000"; -- 9

            -- Blank the display for non-BCD codes
            when others => return "1111111";
        end case;
    end function;


begin

    ------------------------------------------------------------
    -- Clock generation
    ------------------------------------------------------------

    SDRAM_PLL_INST : sdram_pll
        port map (
            inclk0 => CLOCK_50,
            c0     => DRAM_CLK,
            c1     => system_clock
        );


    ------------------------------------------------------------
    -- Nios II system instance
    ------------------------------------------------------------

    NiosII : nios_system
        port map (
            clk     => system_clock,
            reset_n => KEY(0),

            -- SDRAM
            zs_addr_from_the_sdram      => DRAM_ADDR,
            zs_ba_from_the_sdram        => dram_ba_s,
            zs_cas_n_from_the_sdram     => DRAM_CAS_N,
            zs_cke_from_the_sdram       => DRAM_CKE,
            zs_cs_n_from_the_sdram      => DRAM_CS_N,
            zs_dq_to_and_from_the_sdram => DRAM_DQ,
            zs_dqm_from_the_sdram       => dram_dqm_s,
            zs_ras_n_from_the_sdram     => DRAM_RAS_N,
            zs_we_n_from_the_sdram      => DRAM_WE_N,

            -- I2C interface
            i2c_reg_in_port  => I2Cin,
            i2c_reg_out_port => I2Cout,

            -- LED
            ledr_export => LEDR,

            -- 16-bit Avalon PIO
            hex_pio_external_connection_export => temperature_bcd
        );


    ------------------------------------------------------------
    -- SDRAM connections
    ------------------------------------------------------------

    DRAM_BA   <= dram_ba_s;
    DRAM_UDQM <= dram_dqm_s(1);
    DRAM_LDQM <= dram_dqm_s(0);


    ------------------------------------------------------------
    -- MCP9808 I2C interface
    ------------------------------------------------------------

    -- Bit 1 is SCL
    -- Bit 0 is SDA

    I2Cin <= "000000" & GPIO_1(11) & GPIO_1(12);

    -- I2C clock
    GPIO_1(11) <= I2Cout(1);

    -- Open-drain SDA
    GPIO_1(12) <= 'Z' when I2Cout(0) = '1' else '0';


    ------------------------------------------------------------
    -- Temperature on four HEX digits
    ------------------------------------------------------------

    -- Example
    -- Temperature = 23.50 degrees C
    -- Value written by Nios II = 0x2350
    --
    -- HEX3 = 2
    -- HEX2 = 3
    -- HEX1 = 5
    -- HEX0 = 0

    HEX0 <= hex7seg(temperature_bcd(3 downto 0));

    HEX1 <= hex7seg(temperature_bcd(7 downto 4));

    HEX2 <= hex7seg(temperature_bcd(11 downto 8));

    HEX3 <= hex7seg(temperature_bcd(15 downto 12));


    ------------------------------------------------------------
    -- Unused HEX4 and HEX5 displays
    ------------------------------------------------------------

    HEX4 <= (others => '1');
    HEX5 <= (others => '1');

end rtl;