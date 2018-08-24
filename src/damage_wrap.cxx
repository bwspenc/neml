#include "pyhelp.h" // include first to avoid annoying redef warning

#include "damage.h"

#include "nemlerror.h"

#include "pybind11/pybind11.h"
#include "pybind11/numpy.h"
#include "pybind11/stl.h"

namespace py = pybind11;

PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>);

namespace neml {

PYBIND11_MODULE(damage, m) {
  m.doc() = "NEML damage models.";

  py::class_<NEMLDamagedModel_sd, NEMLModel_sd, std::shared_ptr<NEMLDamagedModel_sd>>(m, "NEMLDamagedModel_sd")
      .def_property_readonly("ndamage", &NEMLDamagedModel_sd::ndamage, "Number of damage variables.")
      .def("init_damage",
           [](NEMLDamagedModel_sd & m) -> py::array_t<double>
           {
            auto h = alloc_vec<double>(m.ndamage());
            int ier = m.init_damage(arr2ptr<double>(h));
            py_error(ier);
            return h;
           }, "Initialize damage variables.")
      ;

  py::class_<SDTrialState, TrialState>(m, "SDTrialState")
      ;

  py::class_<NEMLScalarDamagedModel_sd, NEMLDamagedModel_sd, Solvable, std::shared_ptr<NEMLScalarDamagedModel_sd>>(m, "NEMLScalarDamagedModel_sd")
      .def("damage",
           [](NEMLScalarDamagedModel_sd & m, double d_np1, double d_n, py::array_t<double, py::array::c_style> e_np1, py::array_t<double, py::array::c_style> e_n, py::array_t<double, py::array::c_style> s_np1, py::array_t<double, py::array::c_style> s_n, double T_np1, double T_n, double t_np1, double t_n) -> double
           {
            double damage;
            int ier = m.damage(d_np1, d_n, arr2ptr<double>(e_np1), arr2ptr<double>(e_n),
                     arr2ptr<double>(s_np1), arr2ptr<double>(s_n),
                     T_np1, T_n,
                     t_np1, t_n, &damage);
            py_error(ier);
            return damage;
           }, "The damage evolution equation.")
      .def("ddamage_dd",
           [](NEMLScalarDamagedModel_sd & m, double d_np1, double d_n, py::array_t<double, py::array::c_style> e_np1, py::array_t<double, py::array::c_style> e_n, py::array_t<double, py::array::c_style> s_np1, py::array_t<double, py::array::c_style> s_n, double T_np1, double T_n, double t_np1, double t_n) -> double
           {
            double ddamage;
            int ier = m.ddamage_dd(d_np1, d_n, arr2ptr<double>(e_np1), arr2ptr<double>(e_n),
                     arr2ptr<double>(s_np1), arr2ptr<double>(s_n),
                     T_np1, T_n,
                     t_np1, t_n, &ddamage);
            py_error(ier);
            return ddamage;
           }, "The derivative of the damage evolution equation wrt. damage.")
      .def("ddamage_de",
           [](NEMLScalarDamagedModel_sd & m, double d_np1, double d_n, py::array_t<double, py::array::c_style> e_np1, py::array_t<double, py::array::c_style> e_n, py::array_t<double, py::array::c_style> s_np1, py::array_t<double, py::array::c_style> s_n, double T_np1, double T_n, double t_np1, double t_n) -> py::array_t<double>
           {
            auto ddamage = alloc_vec<double>(6);
            int ier = m.ddamage_de(d_np1, d_n, arr2ptr<double>(e_np1), arr2ptr<double>(e_n),
                     arr2ptr<double>(s_np1), arr2ptr<double>(s_n),
                     T_np1, T_n,
                     t_np1, t_n, arr2ptr<double>(ddamage));
            py_error(ier);
            return ddamage;
           }, "The derivative of the damage evolution equation wrt. strain.") 
      .def("ddamage_ds",
           [](NEMLScalarDamagedModel_sd & m, double d_np1, double d_n, py::array_t<double, py::array::c_style> e_np1, py::array_t<double, py::array::c_style> e_n, py::array_t<double, py::array::c_style> s_np1, py::array_t<double, py::array::c_style> s_n, double T_np1, double T_n, double t_np1, double t_n) -> py::array_t<double>
           {
            auto ddamage = alloc_vec<double>(6);
            int ier = m.ddamage_ds(d_np1, d_n, arr2ptr<double>(e_np1), arr2ptr<double>(e_n),
                     arr2ptr<double>(s_np1), arr2ptr<double>(s_n),
                     T_np1, T_n,
                     t_np1, t_n, arr2ptr<double>(ddamage));
            py_error(ier);
            return ddamage;
           }, "The derivative of the damage evolution equation wrt. stress.") 
      .def("make_trial_state",
           [](NEMLScalarDamagedModel_sd & m, py::array_t<double, py::array::c_style> e_np1, py::array_t<double, py::array::c_style> e_n, double T_np1, double T_n, double t_np1, double t_n, py::array_t<double, py::array::c_style> s_n, py::array_t<double, py::array::c_style> h_n, double u_n, double p_n) -> std::unique_ptr<SDTrialState>
           {
            std::unique_ptr<SDTrialState> ts(new SDTrialState);
            int ier = m.make_trial_state(arr2ptr<double>(e_np1),
                                         arr2ptr<double>(e_n),
                                         T_np1, T_n, t_np1, t_n,
                                         arr2ptr<double>(s_n),
                                         arr2ptr<double>(h_n),
                                         u_n, p_n, *ts);
            py_error(ier);

            return ts;
           }, "Make a trial state, mostly for testing.")
      ;

  py::class_<CombinedDamageModel_sd, NEMLScalarDamagedModel_sd, std::shared_ptr<CombinedDamageModel_sd>>(m, "CombinedDamageModel_sd")
      .def(py::init<
           std::shared_ptr<LinearElasticModel>,
           std::vector<std::shared_ptr<NEMLScalarDamagedModel_sd>>,
           std::shared_ptr<NEMLModel_sd>,
           std::shared_ptr<Interpolate>, double, int, bool>(),
            py::arg("elastic"),
            py::arg("models"),
            py::arg("base"), py::arg("alpha") = nullptr, 
            py::arg("tol") = 1.0e-8, py::arg("miter") = 50,
            py::arg("verbose") = false)
      .def(py::init<
           std::shared_ptr<LinearElasticModel>,
           std::vector<std::shared_ptr<NEMLScalarDamagedModel_sd>>,
           std::shared_ptr<NEMLModel_sd>,
           double, double, int, bool>(),
            py::arg("elastic"),
            py::arg("models"),
            py::arg("base"), py::arg("alpha") = 0.0, 
            py::arg("tol") = 1.0e-8, py::arg("miter") = 50,
            py::arg("verbose") = false)
      ;

  py::class_<ClassicalCreepDamageModel_sd, NEMLScalarDamagedModel_sd, std::shared_ptr<ClassicalCreepDamageModel_sd>>(m, "ClassicalCreepDamageModel_sd")
      .def(py::init<
           std::shared_ptr<LinearElasticModel>,
           std::shared_ptr<Interpolate>, std::shared_ptr<Interpolate>,
           std::shared_ptr<Interpolate>, std::shared_ptr<NEMLModel_sd>,
           std::shared_ptr<Interpolate>, double, int, bool>(),
            py::arg("elastic"),
            py::arg("A"), py::arg("xi"), py::arg("phi"),
            py::arg("base"), py::arg("alpha") = nullptr, 
            py::arg("tol") = 1.0e-8, py::arg("miter") = 50,
            py::arg("verbose") = false)
      .def(py::init<
           std::shared_ptr<LinearElasticModel>,
           double, double,
           double, std::shared_ptr<NEMLModel_sd>,
           double, double, int, bool>(),
            py::arg("elastic"),
            py::arg("A"), py::arg("xi"), py::arg("phi"),
            py::arg("base"), py::arg("alpha") = 0.0, 
            py::arg("tol") = 1.0e-8, py::arg("miter") = 50,
            py::arg("verbose") = false)
      ;

  py::class_<MarkFatigueDamageModel_sd, NEMLScalarDamagedModel_sd, std::shared_ptr<MarkFatigueDamageModel_sd>>(m, "MarkFatigueDamageModel_sd")
      .def(py::init<
           std::shared_ptr<LinearElasticModel>,
           std::shared_ptr<Interpolate>, std::shared_ptr<Interpolate>, std::shared_ptr<Interpolate>,
           std::shared_ptr<Interpolate>, std::shared_ptr<Interpolate>,
           std::shared_ptr<Interpolate>,
           std::shared_ptr<NEMLModel_sd>, std::shared_ptr<Interpolate>, double, int, bool>(),
            py::arg("elastic"), py::arg("C"), py::arg("m"), py::arg("n"), py::arg("falpha"), py::arg("fbeta"), py::arg("rate0"),
            py::arg("base"), py::arg("alpha") = nullptr, py::arg("tol") = 1.0e-8, py::arg("miter") = 50, 
            py::arg("verbose") = false)
      .def(py::init<
           std::shared_ptr<LinearElasticModel>,
           double, double, double,
           double, double, double,
           std::shared_ptr<NEMLModel_sd>, double, double, int, bool>(),
            py::arg("elastic"), py::arg("C"), py::arg("m"), py::arg("n"), py::arg("falpha"), py::arg("fbeta"), py::arg("rate0"),
            py::arg("base"), py::arg("alpha") = 0.0, py::arg("tol") = 1.0e-8, py::arg("miter") = 50, 
            py::arg("verbose") = false)
      ;

  py::class_<NEMLStandardScalarDamagedModel_sd, NEMLScalarDamagedModel_sd, std::shared_ptr<NEMLStandardScalarDamagedModel_sd>>(m, "NEMLStandardScalarDamagedModel_sd")
      .def("f",
           [](NEMLStandardScalarDamagedModel_sd & m, py::array_t<double, py::array::c_style> s_np1, double d_np1, double T_np1) -> double
           {
            double fv;
            int ier = m.f(arr2ptr<double>(s_np1), d_np1, T_np1, fv);
            py_error(ier);
            
            return fv;
           }, "The damage evolution function.")
      .def("df_ds",
           [](NEMLStandardScalarDamagedModel_sd & m, py::array_t<double, py::array::c_style> s_np1, double d_np1, double T_np1) -> py::array_t<double>
           {
            auto dfv = alloc_vec<double>(6);
            int ier = m.df_ds(arr2ptr<double>(s_np1), d_np1, T_np1, arr2ptr<double>(dfv));
            py_error(ier);

            return dfv;
           }, "The derivative of the damage function wrt. stress.")
      .def("df_dd",
           [](NEMLStandardScalarDamagedModel_sd & m, py::array_t<double, py::array::c_style> s_np1, double d_np1, double T_np1) -> double
           {
            double dfv;
            int ier = m.df_dd(arr2ptr<double>(s_np1), d_np1, T_np1, dfv);
            py_error(ier);
            
            return dfv;
           }, "The derivative of the damage function wrt. damage")

      ;

  py::class_<NEMLPowerLawDamagedModel_sd, NEMLStandardScalarDamagedModel_sd, std::shared_ptr<NEMLPowerLawDamagedModel_sd>>(m, "NEMLPowerLawDamagedModel_sd")
      .def(py::init<
           std::shared_ptr<LinearElasticModel>,
           std::shared_ptr<Interpolate>, std::shared_ptr<Interpolate>,
           std::shared_ptr<NEMLModel_sd>,
           std::shared_ptr<Interpolate>, double, int, bool>(),
            py::arg("elastic"), py::arg("A"), py::arg("a"), py::arg("base"),
            py::arg("alpha") = nullptr, py::arg("tol") = 1.0e-8, py::arg("miter") = 50, 
            py::arg("verbose") = false)
      .def(py::init<
           std::shared_ptr<LinearElasticModel>, double, double,
           std::shared_ptr<NEMLModel_sd>,
           double, double, int, bool>(),
            py::arg("elastic"), py::arg("A"), py::arg("a"), py::arg("base"), 
            py::arg("alpha") = 0.0, py::arg("tol") = 1.0e-8, py::arg("miter") = 50, 
            py::arg("verbose") = false)
      ;

  py::class_<NEMLExponentialWorkDamagedModel_sd, NEMLStandardScalarDamagedModel_sd, std::shared_ptr<NEMLExponentialWorkDamagedModel_sd>>(m, "NEMLExponentialWorkDamagedModel_sd")
      .def(py::init<
           std::shared_ptr<LinearElasticModel>,
           std::shared_ptr<Interpolate>, std::shared_ptr<Interpolate>,
           std::shared_ptr<Interpolate>,
           std::shared_ptr<NEMLModel_sd>,
           std::shared_ptr<Interpolate>, double, int, bool>(),
            py::arg("elastic"), py::arg("W0"), py::arg("k0"), py::arg("af"), py::arg("base"), 
            py::arg("alpha") = nullptr, py::arg("tol") = 1.0e-8, py::arg("miter") = 50, 
            py::arg("verbose") = false)
      .def(py::init<
           std::shared_ptr<LinearElasticModel>,
           double, double, double,
           std::shared_ptr<NEMLModel_sd>,
           double, double, int, bool>(),
            py::arg("elastic"), py::arg("W0"), py::arg("k0"), py::arg("af"), py::arg("base"), 
            py::arg("alpha") = 0.0, py::arg("tol") = 1.0e-8, py::arg("miter") = 50, 
            py::arg("verbose") = false)
      ;
}

} //  namespace neml
