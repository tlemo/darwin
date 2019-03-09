
## Feb 23rd, 2019

Various additions to the CGP implementation:
- [CGP genotype exporter](scripts/docs/scripts.md#cgp_genotype_exporterpy)
- Support for multiple selection algorithms
- Evolvable constants
- Stateful functions
- A simple one-point crossover operator

## Feb 1st, 2019

New population: a basic [Cartesian Genetic Programming (CGP)](https://www.cartesiangp.com)
implementation.

## Jan 17th, 2019

Yet another take on inverted pendulums: this time the pole is attached to a wheel
and the agent tries to keep the pole balanced by applying torque to the wheel, similar
to riding an unicycle.

It also introduces an additional goal: keep the unicycle close to a random target
position. This is rewarded by a fitness bonus inversely proportional to the absolute
distance from the target (but only if the pole is balanced for the whole episode)

![Unicycle screenshot](docs/images/unicycle_sandbox.png)

## Jan 16th, 2019

New domain: [double-cart-pole][4]. This is a more difficult variation of the single-pole
balancing problem.

![Double-Cart-Pole screenshot](docs/images/double_cart_pole_sandbox.png)

## Jan 5th, 2019

New domain: the classic cart-pole control theory benchmark problem (also known as “pole
balancing” or “inverted pendulum”)

![Cart-Pole screenshot](docs/images/cart_pole_sandbox.png)

This is the first domain implementation to take advantage of third_party/box2d. Using a 
complex physics engine for this problem is arguably an overkill, although it does allow
a wide range of interesting variations. For example the current implementation exposes the
following configuration knobs:

- world gravity
- pole length and density
- cart density and friction
- discrete vs continuous force applications

For more information, see the cart_pole domain
[documentation][3].

## Dec 27th, 2018

Adding experimental support for the [Box2d](http://box2d.org) physics engine 
(third_party/box2d).

## Dec 21st, 2018

Updating and restructuring the documentation.

## Dec 9th, 2018

Added a rudimentary support for batching experiment runs in Darwin Studio. The
"experiment run dialog" allows setting the number of batch runs (default is 1) and
also a `max_generations` limit:

![Fitness Values](docs/images/batch_run_dialog.png)

This has the same effect as repeatedly running the same experiment variation multiple
times, and the result of each run is recorded as a separate `trace` pointing to the same
parent variation.

The aggregated results can be visualized using the new 
[batch_fitness.py](scripts/docs/scripts.md#batch_fitnesspy) script.

## Nov 23rd, 2018

Added [core::PropertySetVariant<TAG>][1] and the corresponding support in
[core_ui::PropertiesWidget][2]:

![PropertySetVariant editor](docs/images/property_set_variant.png)

The intention is to allow sub-modules (for example multime tournament 
implementations) to define their specific configurations.

## Nov 11th, 2018

Initial release

[1]: https://tlemo.github.io/darwin/classcore_1_1_property_set_variant.html
[2]: https://tlemo.github.io/darwin/classcore__ui_1_1_properties_widget.html
[3]: https://tlemo.github.io/darwin/classcart__pole_1_1_cart_pole.html
[4]: https://tlemo.github.io/darwin/classdouble__cart__pole_1_1_double_cart_pole.html
