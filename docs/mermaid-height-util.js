/**
 * @file mermaid-height-util.js
 * @brief Utility for adjusting Mermaid diagram heights and scale controls
 * 
 * This utility provides interactive scaling and height adjustment
 * for Mermaid diagrams in documentation pages.
 * 
 * @author Secure IoT Team
 * @version 1.0.0
 */

class MermaidHeightAdjuster {
    constructor() {
        this.currentScale = 1.0;
        this.minScale = 0.5;
        this.maxScale = 2.0;
        this.init();
    }

    init() {
        this.setupTabSwitching();
        this.setupScaleControls();
        this.adjustAllDiagrams();
    }

    setupTabSwitching() {
        const tabs = document.querySelectorAll('.nav-tab');
        tabs.forEach(tab => {
            tab.addEventListener('click', (e) => this.showTab(e));
        });
    }

    setupScaleControls() {
        document.addEventListener('click', (e) => {
            if (e.target.matches('.scale-btn[data-scale-delta]')) {
                this.changeScale(parseFloat(e.target.dataset.scaleDelta));
            } else if (e.target.matches('.scale-btn[data-reset-scale]')) {
                this.resetScale();
            }
        });
    }

    showTab(event) {
        const tabName = event.currentTarget.dataset.tab;
        
        // Hide all tabs
        document.querySelectorAll('.tab-pane').forEach(tab => {
            tab.classList.remove('active');
        });
        
        // Remove active class from buttons
        document.querySelectorAll('.nav-tab').forEach(btn => {
            btn.classList.remove('active');
        });
        
        // Show selected tab
        const targetTab = document.getElementById(tabName);
        if (targetTab) {
            targetTab.classList.add('active');
            this.adjustDiagramsInTab(targetTab);
        }
        
        // Activate button
        event.currentTarget.classList.add('active');
    }

    changeScale(delta) {
        this.currentScale = Math.max(this.minScale, 
            Math.min(this.maxScale, this.currentScale + delta));
        this.updateScaleDisplay();
        this.adjustAllDiagrams();
    }

    resetScale() {
        this.currentScale = 1.0;
        this.updateScaleDisplay();
        this.adjustAllDiagrams();
    }

    updateScaleDisplay() {
        const displays = document.querySelectorAll('.scale-display');
        displays.forEach(display => {
            display.textContent = Math.round(this.currentScale * 100) + '%';
        });
    }

    adjustDiagramsInTab(tab) {
        const containers = tab.querySelectorAll('.diagram-container');
        containers.forEach(container => this.adjustDiagram(container));
    }

    adjustAllDiagrams() {
        const containers = document.querySelectorAll('.diagram-container');
        containers.forEach(container => this.adjustDiagram(container));
    }

    adjustDiagram(container) {
        const svg = container.querySelector('svg');
        if (!svg) return;

        const bbox = svg.getBBox();
        const padding = 40;
        const scaledHeight = (bbox.height + padding) * this.currentScale;
        const scaledWidth = (bbox.width + padding) * this.currentScale;
        
        // Apply scaling
        svg.style.transform = `scale(${this.currentScale})`;
        svg.style.transformOrigin = 'center';
        
        // Update container dimensions
        container.style.height = Math.min(scaledHeight, window.innerHeight * 0.8) + 'px';
        container.style.width = '100%';
        container.style.minWidth = Math.min(scaledWidth + 100, 1200) + 'px';
        
        // Update wrapper width for proper scrolling
        const wrapper = container.querySelector('.mermaid-wrapper');
        if (wrapper) {
            wrapper.style.width = Math.min(scaledWidth + 100, 1200) + 'px';
        }
    }
}

// Initialize when DOM is ready
document.addEventListener('DOMContentLoaded', () => {
    window.mermaidHeightAdjuster = new MermaidHeightAdjuster();
});
