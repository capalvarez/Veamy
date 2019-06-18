#include <vector>
#include <delynoi/models/basic/Point.h>
#include <delynoi/models/Region.h>
#include <delynoi/models/hole/CircularHole.h>
#include <delynoi/models/generator/functions/functions.h>
#include <delynoi/voronoi/TriangleVoronoiGenerator.h>
#include <veamy/Veamer.h>
#include <chrono>
#include <veamy/models/constraints/values/Constant.h>
#include <utilities/utilities.h>
#include <veamy/physics/materials/MaterialPlaneStrain.h>
#include <veamy/config/VeamyConfig.h>
#include <veamy/physics/conditions/LinearElasticityConditions.h>
#include <veamy/problems/VeamyLinearElasticityDiscretization.h>

int main(){
    // Set precision for plotting to output files:   
    // OPTION 1: in "VeamyConfig::instance()->setPrecision(Precision::precision::mid)"    
    // use "small" for 6 digits; "mid" for 10 digits; "large" for 16 digits.    
    // OPTION 2: set the desired precision, for instance, as:    
    // VeamyConfig::instance()->setPrecision(12) for 12 digits. Change "12" by the desired precision.    
    // OPTION 3: Omit any instruction "VeamyConfig::instance()->setPrecision(.....)"    
    // from this file. In this case, the default precision, which is 6 digits, will be used.   
    VeamyConfig::instance()->setPrecision(Precision::precision::mid);    
    
    // DEFINING PATH FOR THE OUTPUT FILES:
    // If the path for the output files is not given, they are written to /home/user/ directory by default.
    // Otherwise, include the path. For instance, for /home/user/Documents/Veamy-3.0/output.txt , the path
    // must be "Documents/Veamy-3.0/output.txt"
    // CAUTION: the path must exists either because it is already in your system or becuase it is created
    // by Veamy's configuration files. For instance, Veamy creates the folder "/test" inside "/build", so
    // one can save the output files to "/build/test/" folder, but not to "/build/test/mycustom_folder",
    // since "/mycustom_folder" won't be created by Veamy's configuration files.
    std::string meshFileName = "cook_membrane_mesh.txt";
    std::string dispFileName = "cook_membrane_displacements.txt";
    std::string geoFileName = "cook_membrane_geometry.txt";
    
    std::cout << "*** Starting Veamy ***" << std::endl;
    std::cout << "--> Test: Cook's membrane <--" << std::endl;
    std::cout << "..." << std::endl;
    
    std::cout << "+ Defining the domain ... ";
    std::vector<Point> TBeam_points = {Point(0,0), Point(48,44), Point(48,64), Point(0,44)};
    Region TBeam(TBeam_points);

    Hole hole1 = CircularHole(Point(8,30), 5);
    Hole hole2 = CircularHole(Point(24,40), 4);
    Hole hole3 = CircularHole(Point(40,50), 3);
    TBeam.addHole(hole1);
    TBeam.addHole(hole2);
    TBeam.addHole(hole3);
    std::cout << "done" << std::endl;
    
    std::cout << "+ Printing geometry to a file ... ";
    TBeam.printInFile(geoFileName);
    std::cout << "done" << std::endl;    

    std::cout << "+ Generating polygonal mesh ... ";
    TBeam.generateSeedPoints(PointGenerator(functions::constantAlternating(), functions::constant()), 16, 16);
    std::vector<Point> seeds = TBeam.getSeedPoints();
    TriangleVoronoiGenerator g(seeds, TBeam);
    Mesh<Polygon> mesh = g.getMesh();
    std::cout << "done" << std::endl;

    std::cout << "+ Printing mesh to a file ... ";
    mesh.printInFile(meshFileName);
    std::cout << "done" << std::endl;

    std::cout << "+ Defining linear elastic material ... ";
    Material* material = new MaterialPlaneStrain(240, 0.3);
    LinearElasticityConditions* conditions = new LinearElasticityConditions(material);
    std::cout << "done" << std::endl;

    std::cout << "+ Defining Dirichlet and Neumann boundary conditions ... ";
    PointSegment leftSide(Point(0,0), Point(0,44));
    SegmentConstraint left(leftSide, mesh.getPoints(), new Constant(0));
    conditions->addEssentialConstraint(left, mesh.getPoints(), elasticity_constraints::Direction::Total);
    PointSegment rightSide(Point(48,44), Point(48,64));
    SegmentConstraint right(rightSide, mesh.getPoints(), new Constant(6.25));
    conditions->addNaturalConstraint(right, mesh.getPoints(), elasticity_constraints::Direction::Vertical);
    std::cout << "done" << std::endl;

    std::cout << "+ Preparing the simulation ... ";
    VeamyLinearElasticityDiscretization* problem = new VeamyLinearElasticityDiscretization(conditions);
    Veamer v(problem);
    v.initProblem(mesh);
    std::cout << "done" << std::endl;

    std::cout << "+ Simulating ... ";
    std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
    Eigen::VectorXd x = v.simulate(mesh);
    std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
    std::cout << "done" << std::endl;
    std::cout << "  Elapsed simulation time: " << duration/1e6 << " s" <<std::endl;

    std::cout << "+ Printing nodal displacement solution to a file ... ";
    v.writeDisplacements(dispFileName, x);
    std::cout << "done" << std::endl;
    std::cout << "+ Problem finished successfully" << std::endl;
    std::cout << "..." << std::endl;
    std::cout << "Check output files:" << std::endl;
    std::string path1 = utilities::getPath();
    std::string path2 = utilities::getPath();
    std::string path3 = utilities::getPath();
    path1 +=  meshFileName;
    path2 +=  dispFileName;
    path3 +=  geoFileName;
    std::cout << path1 << std::endl;
    std::cout << path2 << std::endl;
    std::cout << path3 << std::endl;
    std::cout << "*** Veamy has ended ***" << std::endl;
}
