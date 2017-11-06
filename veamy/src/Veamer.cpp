#include <veamy/Veamer.h>
#include <iomanip>
#include <veamy/models/constraints/values/Constant.h>
#include <veamy/postprocess/integrator/VeamyIntegrator.h>

Veamer::Veamer() {}

Mesh<Polygon> Veamer::initProblemFromFile(std::string fileName, Material* material) {
    return initProblemFromFile(fileName, material, new None());
}

Mesh<Polygon> Veamer::initProblemFromFile(std::string fileName, Material* material, BodyForce* force) {
    Mesh<Polygon> mesh;
    std::string completeName = utilities::getPath() + fileName;
    std::ifstream infile(completeName);
    if(!infile){
        throw std::invalid_argument("Error: file not found. Check the path to the file.");
    }

    mesh.createFromStream(infile);

    EssentialConstraints essential;
    std::vector<Point> constrainedPointsX;
    std::vector<Point> constrainedPointsY;
    std::vector<Point> constrainedPointsXY;

    std::string line;
    std::getline(infile, line);
    int numberEssential = std::atoi(line.c_str());
    for (int i = 0; i < numberEssential; ++i) {
        std::getline(infile, line);
        std::vector<std::string> splittedLine = utilities::splitBySpaces(line);

        if(splittedLine[1] == "1" && splittedLine[2] == "1"){
            Point p = mesh.getPoint(std::atoi(splittedLine[0].c_str()) - 1);
            constrainedPointsXY.push_back(p);
        }

        if(splittedLine[1] == "1" && splittedLine[2] == "0"){
            Point p = mesh.getPoint(std::atoi(splittedLine[0].c_str()) - 1);
            constrainedPointsX.push_back(p);
        }

        if(splittedLine[1] == "0" && splittedLine[2] == "1"){
            Point p = mesh.getPoint(std::atoi(splittedLine[0].c_str()) - 1);
            constrainedPointsY.push_back(p);
        }
    }

    PointConstraint xConstrained (constrainedPointsX, Constraint::Direction::Horizontal, new Constant(0));
    PointConstraint yConstrained (constrainedPointsY, Constraint::Direction::Vertical, new Constant(0));
    PointConstraint xyConstrained (constrainedPointsXY, Constraint::Direction::Total, new Constant(0));
    essential.addConstraint(xConstrained);
    essential.addConstraint(yConstrained);
    essential.addConstraint(xyConstrained);

    NaturalConstraints natural;
    std::getline(infile, line);
    int numberNatural = std::atoi(line.c_str());
    for (int i = 0; i < numberNatural; ++i) {
        std::getline(infile, line);
        std::vector<std::string> splittedLine = utilities::splitBySpaces(line);

        Point p = mesh.getPoint(std::atoi(splittedLine[0].c_str()) - 1);
        PointConstraint xConstraint(p, Constraint::Direction::Horizontal, new Constant(std::atof(splittedLine[1].c_str())));
        PointConstraint yConstraint(p, Constraint::Direction::Vertical, new Constant(std::atof(splittedLine[2].c_str())));

        natural.addConstraint(xConstraint);
        natural.addConstraint(yConstraint);
    }

    infile.close();
    ConstraintsContainer container;
    container.addConstraints(natural, mesh.getPoints());
    container.addConstraints(essential, mesh.getPoints());

    Conditions conditions(container, force, material);
    initProblem(mesh, conditions);

    return mesh;
}


void Veamer::initProblem(Mesh<Polygon> m, Conditions conditions) {
    std::vector<Point> meshPoints = m.getPoints().getList();
    this->points.push_list(meshPoints);
    this->conditions = conditions;

    std::vector<Polygon> polygons = m.getPolygons();

    for(int i=0;i<polygons.size();i++){
        this->elements.push_back(VemElement(this->conditions, polygons[i], this->points, DOFs));
    }
}

void Veamer::createAndAssemble(Eigen::MatrixXd &KGlobal, Eigen::VectorXd &fGlobal) {
    for(int i=0;i<elements.size();i++){
        elements[i].computeK(DOFs, this->points, conditions);
        elements[i].computeF(DOFs, this->points, conditions);
        elements[i].assemble(DOFs, KGlobal, fGlobal);
    }
}

double Veamer::computeErrorNorm(NormCalculator *calculator, Mesh<Polygon> mesh) {
    calculator->setCalculators(new VeamyIntegrator, new VeamyIntegrator);
    return calculator->getNorm(mesh);
}

