library ieee;
use ieee.std_logic_1164.all;
library altera_mf;
use altera_mf.altera_mf_components.all;

entity sdram_pll is
    port (
        inclk0 : in  std_logic;
        c0     : out std_logic;
        c1     : out std_logic
    );
end sdram_pll;

architecture rtl of sdram_pll is
    signal clk_bus : std_logic_vector(5 downto 0);
begin
    pll_inst : altpll
        generic map (
            operation_mode          => "NORMAL",
            inclk0_input_frequency  => 20000,
            clk0_multiply_by        => 1,
            clk0_divide_by          => 1,
            clk0_phase_shift        => "-3000",
            clk1_multiply_by        => 1,
            clk1_divide_by          => 1,
            clk1_phase_shift        => "0",
            compensate_clock        => "CLK0",
            intended_device_family  => "Cyclone V",
            lpm_type                => "altpll"
        )
        port map (
            inclk => inclk0 & inclk0,
            clk   => clk_bus
        );

    c0 <= clk_bus(0);
    c1 <= clk_bus(1);
end rtl;
