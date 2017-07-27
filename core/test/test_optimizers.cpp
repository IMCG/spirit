#include <catch.hpp>
#include <Spirit/State.h>
#include <Spirit/Configurations.h>
#include <Spirit/Transitions.h>
#include <Spirit/Parameters.h>
#include <Spirit/Geometry.h>
#include <Spirit/Simulation.h>
#include <Spirit/System.h>
#include <Spirit/Chain.h>
#include <Spirit/Quantities.h>
#include <iostream>

TEST_CASE( "Optimizers testing", "[optimizers]" )
{
    // Input file
    auto inputfile = "core/test/input/optimizers.cfg";
    
    // State
    auto state = std::shared_ptr<State>( State_Setup( inputfile ), State_Delete );

    // LLG simulation test
    auto method = "LLG";
    
    // Optimizers to be tested
    std::vector<const char *>  optimizers { "VP", "Heun", "SIB" };
    
    // Expected values
    float energy_expected = -5849.69140625f;
    std::vector<float> magnetization_expected{ 0, 0, 0.79977f };
    
    // Result values
    scalar energy;
    std::vector<float> magnetization{ 0, 0, 0 };
    
    // Calculate energy and magnetization for every optimizer
    for ( auto opt : optimizers )
    {
        // Put a skyrmion in the center of the space
        Configuration_PlusZ( state.get() );
        Configuration_Skyrmion( state.get(), 5, 1, -90, false, false, false);

        // Do simulation
        Simulation_PlayPause( state.get(), method, opt );

        // Save energy and magnetization
        energy = System_Get_Energy( state.get() );
        Quantity_Get_Magnetization( state.get(), magnetization.data() );
            
        // Log the name of the optimizer
        INFO( opt << std::string( " optimizer using " ) << method );

        // Check the values of energy and magnetization
        REQUIRE( energy == Approx( energy_expected ) );
        for (int dim=0; dim<3; dim++)
            REQUIRE( magnetization[dim] == Approx( magnetization_expected[dim] ) );
    }
    
    Chain_Image_to_Clipboard(state.get());
    int noi = 4;
    
    for (int i=1; i<noi; ++i)
      Chain_Insert_Image_After(state.get());

    // GNEB simulation test
    // GNEB calculation test
    method = "GNEB";

    // Optimizers to be tested
    optimizers = { "VP", "Heun" };

    // Expected values
    float energy_sp_expected = -5811.5244140625f;
    std::vector<float> magnetization_sp_expected{ 0, 0, 0.96657f };

    // Result values
    scalar energy_sp;
    std::vector<float> magnetization_sp{ 0, 0, 0 };

    // Calculate energy and magnetization at saddle point for every optimizer
    for ( auto opt : optimizers )
    {
        // Create a skyrmion collapse transition
        Chain_Replace_Image(state.get(), 0);
        Chain_Jump_To_Image(state.get(), noi-1);
        Configuration_PlusZ(state.get());
        Chain_Jump_To_Image(state.get(), 0);
        Transition_Homogeneous(state.get(), 0, noi-1);
    
        // Do simulation
        Simulation_PlayPause( state.get(), method, opt, 2e4 );
        Parameters_Set_GNEB_Image_Type_Automatically( state.get() );
        Simulation_PlayPause( state.get(), method, opt );

        // Get saddle point index
        int i_max = 1;
        float E_max = System_Get_Energy(state.get(), 0);
        
        for (int i=1; i<noi-1; ++i)
            if (System_Get_Energy(state.get(), i) > E_max) i_max = i;
    
        // Save energy and magnetization
        energy_sp = System_Get_Energy( state.get(), i_max );
        Quantity_Get_Magnetization( state.get(), magnetization_sp.data(), i_max );
            
        // Log the name of the optimizer
        INFO( opt << std::string( " optimizer using " ) << method );
        
        // Check the values of energy and magnetization
        REQUIRE( energy_sp == Approx( energy_sp_expected ) );
        for (int dim=0; dim<3; dim++)
            REQUIRE( magnetization_sp[dim] == Approx( magnetization_sp_expected[dim] ) );
    }

}