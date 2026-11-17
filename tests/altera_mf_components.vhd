library ieee;
use ieee.std_logic_1164.all;

-- Test-only declaration of the interface used by sdram_pll.
package altera_mf_components is
    component altpll is
        generic (
            operation_mode          : string;
            inclk0_input_frequency  : natural;
            clk0_multiply_by        : natural;
            clk0_divide_by          : natural;
            clk0_phase_shift        : string;
            clk1_multiply_by        : natural;
            clk1_divide_by          : natural;
            clk1_phase_shift        : string;
            compensate_clock        : string;
            intended_device_family  : string;
            lpm_type                : string
        );
        port (
            inclk : in  std_logic_vector(1 downto 0);
            clk   : out std_logic_vector(5 downto 0)
        );
    end component;
end package;
