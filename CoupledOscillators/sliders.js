
function createScalarParameterSlider(
    controls, enumCode, sliderLabelName, type, spec) {
    let label = document.createElement("label");
    label.for = spec['id']
    label.style = "color:white; font-family:Arial, Helvetica, sans-serif";
    label.textContent = `${sliderLabelName} = ${spec.value}`
    controls.appendChild(label);
    let slider = document.createElement("input");
    slider.type = "range";
    slider.style ="width: 95%;"
    for (let k of Object.keys(spec))
        slider[k] = spec[k];
    slider.value = spec.value;
    controls.appendChild(document.createElement("br"));
    controls.appendChild(slider);
    controls.appendChild(document.createElement("br"));
    slider.addEventListener("input", e => {
        let valueF = Number.parseFloat(e.target.value);
        let valueI = Number.parseInt(e.target.value);
        if (type === "float") {
            label.textContent = `${sliderLabelName} = ${valueF}`
            Module.set_float_param(enumCode, valueF);
        } else if (type === "int") {
            label.textContent = `${sliderLabelName} = ${valueI}`
            Module.set_int_param(enumCode, valueI);
        }
    });
};

gCheckboxXorLists = {};

function createCheckbox(controls, enumCode, name, value, xorListName='') {
    let label = document.createElement("label");
    // label.for = spec['id']
    label.style = "color:white; font-family:Arial, Helvetica, sans-serif";
    label.textContent = `${name}`
    let checkbox = document.createElement("input");
    checkbox.type = "checkbox";
    checkbox.id = `checkbox-${enumCode}`;
    if (xorListName !== '') {
        if (!(xorListName in gCheckboxXorLists))
            gCheckboxXorLists[xorListName] = [checkbox.id];
        else
            gCheckboxXorLists[xorListName].push(checkbox.id);
    }
    // slider.style ="width: 95%;"
    // checkbox.value = value;
    checkbox.checked = value;
    // controls.appendChild(document.createElement("br"));
    controls.appendChild(checkbox);
    controls.appendChild(label);
    controls.appendChild(document.createElement("br"));
    checkbox.addEventListener("input", e => {
        console.log(e.target.checked);
        Module.set_bool_param(enumCode, e.target.checked);
        if (e.target.checked === true && xorListName !== '') {
            for (let id_ of gCheckboxXorLists[xorListName]) {
                if (id_ !== checkbox.id) {
                    let enumCode2 = parseInt(id_.split('-')[1]);
                    Module.set_bool_param(enumCode2, false);
                    document.getElementById(id_).checked = false;
                }
            }
        }
    }
    );
}

let gVecParams = {};

function createVectorParameterSliders(
    controls, enumCode, sliderLabelName, type, spec) {
    let label = document.createElement("label");
    label.style = "color:white; font-family:Arial, Helvetica, sans-serif";
    label.textContent = `${sliderLabelName} = (${spec.value})`
    gVecParams[sliderLabelName] = spec.value;
    controls.appendChild(label);
    controls.appendChild(document.createElement("br"));
    for (let i = 0; i < spec.value.length; i++) {
        let slider = document.createElement("input");
        slider.type = "range";
        slider.style ="width: 95%;"
        for (let k of Object.keys(spec))
            slider[k] = spec[k][i];
        slider.value = spec.value[i];
        controls.appendChild(slider);
        controls.appendChild(document.createElement("br"));
        slider.addEventListener("input", e => {
            let valueF = Number.parseFloat(e.target.value);
            let valueI = Number.parseInt(e.target.value);
            if (type === "Vec2" || 
                type === "Vec3" || type === "Vec4") {
                gVecParams[sliderLabelName][i] = valueF;
                label.textContent 
                    = `${sliderLabelName} = (${gVecParams[sliderLabelName]})`
                Module.set_vec_param(
                    enumCode, spec.value.length, i, valueF);
            } else if (type === "IVec2" || 
                        type === "IVec3" || type === "IVec4") {
                gVecParams[sliderLabelName][i] = valueI;
                label.textContent 
                    = `${sliderLabelName} = (${gVecParams[sliderLabelName]})`
                Module.set_ivec_param(
                    enumCode, spec.value.length, i, valueI);
            }
        });
    }
};

function createSelectionList(
    controls, enumCode, defaultVal, selectionBoxName, textOptions
) {
    let label = document.createElement("label");
    label.style = "color:white; font-family:Arial, Helvetica, sans-serif";
    label.textContent = selectionBoxName;
    controls.appendChild(label);
    controls.appendChild(document.createElement("br"));
    let selector = document.createElement("select");
    for (let i = 0; i < textOptions.length; i++) {
        let option = document.createElement("option");
        option.value = i;
        option.textContent = textOptions[i];
        selector.add(option);
    }
    selector.value = defaultVal;
    selector.addEventListener("change", e =>
        Module.selection_set(
            enumCode, Number.parseInt(e.target.value))
    );
    controls.appendChild(selector);
    controls.appendChild(document.createElement("br"));
}

let gUserParams = {};

function modifyUserSliders(enumCode, variableList) {
    if (!(`${enumCode}` in gUserParams))
        gUserParams[`${enumCode}`] = {}; 
    for (let c of variableList) {
        if (!( c in gUserParams[`${enumCode}`]))
            gUserParams[`${enumCode}`][c] = 1.0;
    }
    let userSliders 
        = document.getElementById(`user-sliders-${enumCode}`);
    userSliders.textContent = ``;
    for (let v of variableList) {
        let label = document.createElement("label");
        label.style = "color:white; font-family:Arial, Helvetica, sans-serif";
        label.textContent = `${v} = ${gUserParams[`${enumCode}`][v]}`;
        userSliders.appendChild(label);
        let slider = document.createElement("input");
        slider.type = "range";
        slider.style = "width: 95%;"
        slider.min = "-5";
        slider.max = "5";
        slider.step = "0.01";
        slider.value = gUserParams[`${enumCode}`][v];
        slider.addEventListener("input", e => {
            let value = Number.parseFloat(e.target.value);
            label.textContent = `${v} = ${value}`;
            gUserParams[`${enumCode}`][v] = value;
            Module.set_user_float_param(enumCode, v, value);
        });
        userSliders.appendChild(document.createElement("br"));
        userSliders.appendChild(slider);
        userSliders.appendChild(document.createElement("br"));
    }
}

function createEntryBoxes(
    controls, enumCode, entryBoxName, count, subLabels
) {
    let label = document.createElement("label");
    label.style = "color:white; font-family:Arial, Helvetica, sans-serif";
    label.textContent = entryBoxName;
    controls.appendChild(label);
    controls.appendChild(document.createElement("br"));
    let entryBoxes = [];
    for (let i = 0; i < count; i++) {
        let entryBox = document.createElement('input');
        entryBox.type = "text";
        entryBox.value = "";
        entryBox.id = `entry-box-${enumCode}-${i}`;
        entryBox.style = "width: 95%;";
        let label = document.createElement("label");
        label.style = "color:white; font-family:Arial, Helvetica, sans-serif";
        label.textContent = `${subLabels[i]}`;
        if (count >= 2) {
            controls.appendChild(label);
            controls.appendChild(document.createElement("br"));
        }
        controls.appendChild(entryBox);
        controls.appendChild(document.createElement("br"));
        entryBoxes.push(entryBox);
        entryBox.addEventListener("input", e =>
            Module.set_string_param(enumCode, i, `${e.target.value}`)
        );
    }
    let userSlidersDiv = document.createElement("div");
    userSlidersDiv.id = `user-sliders-${enumCode}`
    controls.appendChild(userSlidersDiv);

}

function createButton(
    controls, enumCode, buttonName, style=''
) {
    let button = document.createElement("button");
    button.innerText = buttonName;
    if (style !== '')
        button.style = style;
    controls.appendChild(button);
    controls.appendChild(document.createElement("br"));
    button.addEventListener("click", e => Module.button_pressed(enumCode));
}

function createLabel(
    controls, enumCode, labelName, style=''
) {
    let label = document.createElement("label");
    if (style === '')
        label.style = "color:white; font-family:Arial, Helvetica, sans-serif";
    else
        label.style = style;
    label.textContent = `${labelName}`;
    label.id = `label-${enumCode}`;
    controls.appendChild(label);
    controls.appendChild(document.createElement("br"));
}

function editLabel(enumCode, textContent) {
    let idVal = `label-${enumCode}`;
    let label = document.getElementById(idVal);
    label.textContent = textContent;
}

function createLineDivider(controls) {
    let hr = document.createElement("hr");
    hr.style = "color:white;"
    controls.appendChild(hr);
}

let controls = document.getElementById('controls');
createScalarParameterSlider(controls, 1, "Time elapsed per frame", "float", {'value': 0.1, 'min': 0.0, 'max': 10.0, 'step': 0.01});
createScalarParameterSlider(controls, 4, "Number of oscillators", "int", {'value': 64, 'min': 8, 'max': 256});
createSelectionList(controls, 5, 0, "Boundary type", [ "Zero at endpoints",  "Periodic"]);
createLineDivider(controls);
createLabel(controls, 7, "Metropolis algorithm configuration", "color:white; font-family:Arial, Helvetica, sans-serif; font-weight: bold;");
createScalarParameterSlider(controls, 8, "Relative step size", "float", {'value': 0.66, 'min': 0.0, 'max': 1.0, 'step': 0.01});
createLabel(controls, 9, "Acceptance rate", "");
createScalarParameterSlider(controls, 11, "Requested number of Monte Carlo samples", "int", {'value': 20000, 'min': 10, 'max': 100000});
createLineDivider(controls);
createLabel(controls, 13, "Samples display options", "color:white; font-family:Arial, Helvetica, sans-serif; font-weight: bold;");
createScalarParameterSlider(controls, 14, "Brightness", "float", {'value': 0.01, 'min': 0.0, 'max': 0.1, 'step': 0.0001});
createVectorParameterSliders(controls, 15, "Colour 1 (r, g, b)", "Vec3", {'value': [0.0, 0.85, 1.0], 'min': [0.0, 0.0, 0.0], 'max': [1.0, 1.0, 1.0], 'step': [0.002, 0.002, 0.002]});
createVectorParameterSliders(controls, 16, "Colour 2 (r, g, b)", "Vec3", {'value': [0.0, 1.0, 0.0], 'min': [0.0, 0.0, 0.0], 'max': [1.0, 1.0, 1.0], 'step': [0.002, 0.002, 0.002]});
createSelectionList(controls, 17, 1, "Plot type", [ "Lines",  "Scatter",  "Multi-coloured histogram"]);
createCheckbox(controls, 18, "Display samples in normal coordinates", true);
createLineDivider(controls);
createLabel(controls, 20, "Normal mode analytic wave function display", "color:white; font-family:Arial, Helvetica, sans-serif; font-weight: bold;");
createCheckbox(controls, 21, "Colour phase", false);
createScalarParameterSlider(controls, 22, "Brightness", "float", {'value': 1.25, 'min': 0.0, 'max': 10.0, 'step': 0.01});
createLineDivider(controls);
createLabel(controls, 24, "Wave function modification options", "color:white; font-family:Arial, Helvetica, sans-serif; font-weight: bold;");
createCheckbox(controls, 25, "Coherent (Single product of coherent modes)", true, "waveFuncOptions");
createCheckbox(controls, 26, "Squeezed (Single product)", false, "waveFuncOptions");
createCheckbox(controls, 27, "Energy eigenstate (Expect poor Metropolis convergence for highly excited modes. Single product of normal mode eigenstates only.)", false, "waveFuncOptions");
createCheckbox(controls, 28, "Superposition of singly-excited normal modes", false, "waveFuncOptions");
createLabel(controls, 29, "(Will be difficult to differentiate any differences from the ground unless a large number of samples are used.)", "");
createLabel(controls, 30, "If 'Coherent' or 'Squeezed' selected:", "color:white; font-family:Arial, Helvetica, sans-serif; font-weight: bold;");
createSelectionList(controls, 31, 0, "Behaviour when modifying a selected normal mode amplitude expectation value with the mouse cursor:", [ "Change selected while setting others to zero",  "Modify selection only"]);
createLabel(controls, 32, "If 'Squeezed' selected:", "color:white; font-family:Arial, Helvetica, sans-serif; font-weight: bold;");
createScalarParameterSlider(controls, 33, "Global squeezing factor (compared to coherent)", "float", {'value': 1.0, 'min': 0.5, 'max': 10.0, 'step': 0.01});
createScalarParameterSlider(controls, 34, "Squeeze factor for an individual normal mode", "float", {'value': 1.0, 'min': 0.5, 'max': 10.0, 'step': 0.01});
createLabel(controls, 35, "(Click on a normal mode for this slider to take effect)", "");
createLabel(controls, 36, "If 'Energy eigenstate' selected:", "color:white; font-family:Arial, Helvetica, sans-serif; font-weight: bold;");
createCheckbox(controls, 37, "Click on normal mode to add energy", true, "stationaryOptions");
createCheckbox(controls, 38, "Remove energy instead", false, "stationaryOptions");
createLineDivider(controls);
createLabel(controls, 40, "Dispersion relation options", "color:white; font-family:Arial, Helvetica, sans-serif; font-weight: bold;");
createSelectionList(controls, 41, 0, "Preset dispersion relation ω(k)", [ "2*sin((pi/2)*(abs(k)/k_max))",  "pi*(abs(k)/k_max)"]);

