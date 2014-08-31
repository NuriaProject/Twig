Writing test-cases
------------------

All *.json files in this directory are treated as test-cases for NuriaTwig.
This readme shall serve as short introduction on the structure.

As the ending suggests, test-cases are written in JSON format.
The root is a object with the following items:

* variables: A object which will be exposed as variables to the TemplateEngine
* template: See below
* output: The expected output
* error: A string containing the expected error. See below
* skip: Boolean indicating if this test should be skipped

The ```template``` item may be a string containing the template data of the only
template, or a object containing multiple templates (The key being the name and
the value being the template data string). If it's a object, the main template
being rendered is called ```main```.

The ```error``` item is either a empty string if the test should not fail, or
a string in the form of "Component:Error". 'Component' refers to an entry in
Nuria::TemplateError::Component and 'Error' refers to an entry in
Nuria::TemplateError::Error. An example: "Renderer:VariableNotSet"

You can leave out items you don't need - these will be initialised as if their
content would be empty. So an empty object, empty string or 'false' in case of
```skip```.

That's it. Just make sure to let your file end in .json and are registered as
resource (See tst_templateengine_resources.qrc), else the runner won't pick it
up. If a test-case contains JSON syntax errors, the runner will terminate.
