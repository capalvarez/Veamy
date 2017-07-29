#include <gtest/gtest.h>
#include <veamy/models/constraints/Constraint.h>
#include <veamy/models/constraints/values/Constant.h>
#include <veamy/Veamer.h>
#include <mesher/models/Region.h>
#include <mesher/models/generator/functions.h>
#include <mesher/voronoi/TriangleMeshGenerator.h>
#include <veamy/models/constraints/values/Function.h>

double tangencial(double x, double y){
    double P = -1000;
    double D =  4;
    double I = std::pow(D,3)/12;

    return P/(2*I)*(std::pow(D,2)/4-std::pow(y,2));
}

double uX(double x, double y){
    double P = -1000;
    double Ebar = 1e7/(1 - std::pow(0.3,2));
    double vBar = 0.3/(1 - 0.3);
    double D = 4;
    double L = 8;
    double I = std::pow(D,3)/12;


    return -P*y/(6*Ebar*I)*((6*L - 3*x)*x + (2+vBar)*std::pow(y,2) - 3*std::pow(D,2)/2*(1+vBar));
}

double uY(double x, double y){
    double P = -1000;
    double Ebar = 1e7/(1 - std::pow(0.3,2));
    double vBar = 0.3/(1 - 0.3);
    double D = 4;
    double L = 8;
    double I = std::pow(D,3)/12;

    return P/(6*Ebar*I)*(3*vBar*std::pow(y,2)*(L-x) + (3*L-x)*std::pow(x,2));
}

double uXPatch(double x, double y){
    return x;
}

double uYPatch(double x, double y){
    return x + y;
}

TEST(VeamerTest, ParabolicBeamExampleTest){
    std::vector<Point> rectangle4x8_points = {Point(0, -2), Point(8, -2), Point(8, 2), Point(0, 2)};
    Region rectangle4x8(rectangle4x8_points);
    rectangle4x8.generateSeedPoints(PointGenerator(functions::constantAlternating(), functions::constant()), 24, 12);

    std::vector<Point> seeds = rectangle4x8.getSeedPoints();
    TriangleMeshGenerator meshGenerator = TriangleMeshGenerator (seeds, rectangle4x8);
    PolygonalMesh mesh = meshGenerator.getMesh();
    mesh.printInFile("rectangle4x8ConstantAlternating.txt");

    Veamer v;

    EssentialConstraints essential;
    Function* uXConstraint = new Function(uX);
    Function* uYConstraint = new Function(uY);

    PointSegment leftSide(Point(0,-2), Point(0,2));
    Constraint const1 (leftSide, mesh.getPoints(), Constraint::Direction::Horizontal, uXConstraint);
    essential.addConstraint(const1, mesh.getPoints());

    Constraint const2 (leftSide, mesh.getPoints(), Constraint::Direction::Vertical, uYConstraint);
    essential.addConstraint(const2, mesh.getPoints());

    NaturalConstraints natural;

    Function* tangencialLoad = new Function(tangencial);
    PointSegment rightSide(Point(8,-2), Point(8,2));

    Constraint const3 (rightSide, mesh.getPoints(), Constraint::Direction::Vertical, tangencialLoad);
    natural.addConstraint(const3, mesh.getPoints());

    ConstraintsContainer container;
    container.addConstraints(essential, mesh);
    container.addConstraints(natural, mesh);

    Material m(1e7, 0.3);
    ProblemConditions conditions(container, m);

    v.initProblem(mesh, conditions);

    Eigen::VectorXd x = v.simulate(mesh);
    std::string fileName = "displacement.txt";
    v.writeDisplacements(fileName, x);
}


TEST(VeamerTest, ParabolicBeamPatchTest){
    std::vector<Point> rectangle4x8_points = {Point(0, -2), Point(8, -2), Point(8, 2), Point(0, 2)};
    Region rectangle4x8(rectangle4x8_points);
    rectangle4x8.generateSeedPoints(PointGenerator(functions::constantAlternating(), functions::constant()), 48, 24);

    std::vector<Point> seeds = rectangle4x8.getSeedPoints();
    TriangleMeshGenerator meshGenerator = TriangleMeshGenerator (seeds, rectangle4x8);
    PolygonalMesh mesh = meshGenerator.getMesh();
    mesh.printInFile("rectangle4x8ConstantAlternating.txt");

    Veamer v;

    EssentialConstraints essential;
    Function* uXConstraint = new Function(uXPatch);
    Function* uYConstraint = new Function(uYPatch);

    PointSegment leftSide(Point(0,-2), Point(0,2));
    Constraint leftX (leftSide, mesh.getPoints(), Constraint::Direction::Horizontal, uXConstraint);
    essential.addConstraint(leftX, mesh.getPoints());
    Constraint  leftY (leftSide, mesh.getPoints(), Constraint::Direction::Vertical, uYConstraint);
    essential.addConstraint(leftY, mesh.getPoints());

    PointSegment downSide(Point(0,-2), Point(8,-2));
    Constraint downX (downSide, mesh.getPoints(), Constraint::Direction::Horizontal, uXConstraint);
    essential.addConstraint(downX, mesh.getPoints());
    Constraint  downY (downSide, mesh.getPoints(), Constraint::Direction::Vertical, uYConstraint);
    essential.addConstraint(downY, mesh.getPoints());

    PointSegment rightSide(Point(8,-2), Point(8, 2));
    Constraint rightX (rightSide, mesh.getPoints(), Constraint::Direction::Horizontal, uXConstraint);
    essential.addConstraint(rightX, mesh.getPoints());
    Constraint  rightY (rightSide, mesh.getPoints(), Constraint::Direction::Vertical, uYConstraint);
    essential.addConstraint(rightY, mesh.getPoints());

    PointSegment topSide(Point(0, 2), Point(8, 2));
    Constraint topX (topSide, mesh.getPoints(), Constraint::Direction::Horizontal, uXConstraint);
    essential.addConstraint(topX, mesh.getPoints());
    Constraint  topY (topSide, mesh.getPoints(), Constraint::Direction::Vertical, uYConstraint);
    essential.addConstraint(topY, mesh.getPoints());

    ConstraintsContainer container;
    container.addConstraints(essential, mesh);

    Material m(1e7, 0.3);
    ProblemConditions conditions(container, m);

    v.initProblem(mesh, conditions);

    Eigen::VectorXd x = v.simulate(mesh);
    std::string fileName = "displacement.txt";
    v.writeDisplacements(fileName, x);
}